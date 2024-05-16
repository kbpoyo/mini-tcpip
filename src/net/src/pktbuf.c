/**
 * @file pktbuf.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 数据包模块
 * @version 0.1
 * @date 2024-05-13
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "pktbuf.h"

#include "dbg.h"
#include "mblock.h"

static pktblk_t pktblk_pool[PKTBUF_BLK_CNT];  // 数据块池
static mblock_t pktblk_list;  // 数据块链表(管理数据块池)

static pktbuf_t pktbuf_pool[PKTBUF_BUF_CNT];  // 数据包池
static mblock_t pktbuf_list;  // 数据包链表(管理数据包池)

static nlocker_t pkt_locker;  // 数据包模块锁

static inline int blk_tail_free_size(pktblk_t *blk) {
  return (int)((blk->payload + PKTBUF_BLK_SIZE) - (blk->data + blk->data_size));
}

#if DBG_DISP_ENABLED(DBG_PKTBUF)
static void display_check_buf(pktbuf_t *pktbuf) {
  if (pktbuf == (pktbuf_t *)0) {
    dbg_error(DBG_PKTBUF, "invalid pktbuf, pktbuf is null.");
    return;
  }

  plat_printf("check pktbuf: %p, total_size: %d\n", pktbuf, pktbuf->total_size);
  pktblk_t *curr;
  int index = 0, buf_total_size = 0;
  // 遍历数据块列表,检查数据块的有效载荷区域是否正确
  for (curr = pktbuf_blk_first(pktbuf); curr; curr = pktbuf_blk_next(curr)) {
    plat_printf("[%d]:\t", index++);

    // 检测已使用的载荷区域是否在有效载荷区域内
    if (curr->data < curr->payload ||
        curr->data >= curr->payload + PKTBUF_BLK_SIZE) {
      dbg_error(DBG_PKTBUF, "block data error, %p not in [%p, %p)", curr->data,
                curr->payload, curr->payload + PKTBUF_BLK_SIZE);
    }

    int pre_size = (int)(curr->data - curr->payload);
    plat_printf("pre: %d B,\t", pre_size);

    int used_size = curr->data_size;
    plat_printf("used: %d B,\t", used_size);

    int free_size = blk_tail_free_size(curr);
    plat_printf("free: %d B\n", free_size);

    int blk_total_size = pre_size + used_size + free_size;
    if (blk_total_size != PKTBUF_BLK_SIZE) {
      dbg_error(DBG_PKTBUF, "block total size error, %d != %d", blk_total_size,
                PKTBUF_BLK_SIZE);
    }

    buf_total_size += used_size;
  }

  // 检查数据包的总大小是否正确
  if (buf_total_size != pktbuf->total_size) {
    dbg_error(DBG_PKTBUF, "buf total size error, %d != %d", buf_total_size,
              pktbuf->total_size);
  }
}

#else
#define display_check_buf(pktbuf)
#endif

/**
 * @brief 初始化数据包模块
 *
 * @return net_err_t
 */
net_err_t pktbuf_init(void) {
  dbg_info(DBG_PKTBUF, "pktbuf init....");

  nlocker_init(&pkt_locker, PKTBUF_LOCKER_TYPE);
  mblock_init(&pktblk_list, pktblk_pool, sizeof(pktblk_t), PKTBUF_BLK_CNT,
              PKTBUF_LOCKER_TYPE);
  mblock_init(&pktbuf_list, pktbuf_pool, sizeof(pktbuf_t), PKTBUF_BUF_CNT,
              PKTBUF_LOCKER_TYPE);

  dbg_info(DBG_PKTBUF, "pktbuf init success....");
  return NET_ERR_OK;
}

/**
 * @brief 从数据块池中分配一个数据块
 *
 * @return pktblk_t*
 */
static pktblk_t *pktblock_alloc(void) {
  pktblk_t *pktblk = (pktblk_t *)mblock_alloc(&pktblk_list, -1);
  if (pktblk == (pktblk_t *)0) {
    dbg_error(DBG_PKTBUF, "pktblock alloc failed, no buffer.");
    return (pktblk_t *)0;
  }

  pktblk->data_size = 0;
  pktblk->data = (uint8_t *)0;
  nlist_node_init(&(pktblk->node));

  return pktblk;
}

/**
 * @brief 释放一个数据块到数据块池中
 *
 * @param pktblk
 */
static void pktblock_free(pktblk_t *pktblk) {
  mblock_free(&pktblk_list, pktblk);
}

/**
 * @brief 将数据库列表中的数据块释放到数据块池中
 *
 * @param blk_list  数据块列表
 */
static void pktblock_list_free(nlist_t *blk_list) {
  nlist_node_t *blk_node = nlist_first(blk_list);
  nlist_node_t *next_node = (nlist_node_t *)0;
  while (blk_node) {
    // 获取下一个数据块结点
    next_node = nlist_node_next(blk_node);
    // 获取数据块
    pktblk_t *pktblk = nlist_entry(blk_node, pktblk_t, node);
    // 从数据块列表中移除数据块
    nlist_remove(blk_list, blk_node);
    // 将数据块释放到数据块池中
    pktblock_free(pktblk);

    blk_node = next_node;
  }
}

/**
 * @brief 从数据块池中分配一个数据块列表
 *
 * @param blk_list 数据块列表
 * @param size 待分配内存大小
 * @param insert_type 列表插入方式
 * @return pktblk_t*
 */
static net_err_t pktblock_list_alloc(pktbuf_t *pktbuf, int size,
                                     int insert_type) {
  // 创建临时数据块列表
  nlist_t tmp_list;
  nlist_init(&tmp_list);

  // 记录待分配内存大小
  int alloc_size = size;
  while (alloc_size) {
    pktblk_t *newblk = pktblock_alloc();
    if (newblk == (pktblk_t *)0) {
      dbg_error(DBG_PKTBUF, "pktblock alloc failed no buffer for alloc(%d).",
                alloc_size);

      pktblock_list_free(&tmp_list);
      return NET_ERR_MEM;
    }

    // 记录newblk数据块真实负载大小
    int curr_size = alloc_size > PKTBUF_BLK_SIZE ? PKTBUF_BLK_SIZE : alloc_size;
    if (insert_type == PKTBUF_LIST_INSERT_HEAD) {  // 使用头插法
      // 计算当前数据块的有效载荷大小
      newblk->data_size = curr_size;
      newblk->data = newblk->payload + PKTBUF_BLK_SIZE - curr_size;

      nlist_insert_first(&tmp_list, &(newblk->node));
    } else {  // 使用尾插法
      // 计算当前数据块的有效载荷大小
      newblk->data_size = curr_size;
      newblk->data = newblk->payload;
      nlist_insert_last(&tmp_list, &(newblk->node));
    }

    // 更新待分配内存大小
    alloc_size -= curr_size;
  }

  // 将临时数据块列表添加到数据块列表中
  if (insert_type == PKTBUF_LIST_INSERT_HEAD) {  // 使用头插法
    nlist_join(&tmp_list, &(pktbuf->blk_list));
    nlist_move(&(pktbuf->blk_list), &tmp_list);
  } else {  // 使用尾插法
    nlist_join(&(pktbuf->blk_list), &tmp_list);
  }

  pktbuf->total_size += size;

  return NET_ERR_OK;
}

/**
 * @brief 分配一个数据包
 *
 * @param size
 * @return pktbuf_t*
 */
pktbuf_t *pktbuf_alloc(int size) {
  // 分配一个数据包
  pktbuf_t *pktbuf = (pktbuf_t *)mblock_alloc(&pktbuf_list, -1);
  if (pktbuf == (pktbuf_t *)0) {
    dbg_error(DBG_PKTBUF, "pktbuf alloc failed, no buffer.");
    return (pktbuf_t *)0;
  }

  // 初始化数据包
  pktbuf->total_size = 0;
  nlist_init(&(pktbuf->blk_list));
  nlist_node_init(&(pktbuf->node));

  if (size > 0) {
    // 为数据包分配数据块列表
    net_err_t err =
        pktblock_list_alloc(pktbuf, size / 2, PKTBUF_LIST_INSERT_TAIL);

    err = pktblock_list_alloc(pktbuf, size / 2, PKTBUF_LIST_INSERT_HEAD);
    if (err != NET_ERR_OK) {  // 分配失败
      // 释放数据包
      mblock_free(&pktbuf_list, pktbuf);
      return (pktbuf_t *)0;
    }
  }

  display_check_buf(pktbuf);

  return pktbuf;
}

/**
 * @brief 释放数据包对象
 *
 * @param pktbuf
 */
void pktbuf_free(pktbuf_t *pktbuf) {
  if (pktbuf == (pktbuf_t *)0) {
    dbg_error(DBG_PKTBUF, "invalid pktbuf, pktbuf is null.");
    return;
  }

  // 释放数据块列表
  pktblock_list_free(&(pktbuf->blk_list));
  // 释放数据包
  mblock_free(&pktbuf_list, pktbuf);
}

/**
 * @brief 在数据包的头部增加数据块
 *
 * @param buf
 * @param size
 * @param is_cont
 * 若头部数据块剩余空间不够,是否将数据内容全部放入新的数据块中，以保证数据在内存上的连续
 * @return net_err_t
 */
net_err_t pktbuf_add_header(pktbuf_t *buf, int size, int is_cont) {
  // 将当前数据包的第一个数据块剩余空闲空间利用起来
  pktblk_t *block = pktbuf_blk_first(buf);
  // 计算当前数据块头部的剩余空闲空间
  int resv_size = (int)(block->data - block->payload);
  if (size <= resv_size) {
    block->data -= size;
    block->data_size += size;
    buf->total_size += size;

    display_check_buf(buf);  // 检查数据包是否正确
    return NET_ERR_OK;
  }

  // 第一块数据块的剩余空闲空间不够
  // 再为数据包分配数据块列表
  if (is_cont == PKTBUF_ADD_HEADER_CONT) {  // 保证数据在内存上的连续
    // 待分配内存大于数据块大小，无法实现在内存上的连续
    if (size > PKTBUF_BLK_SIZE) {
      dbg_error(DBG_PKTBUF, "can't set cont, size too big: %d > %d.", size,
                PKTBUF_BLK_SIZE);
      return NET_ERR_SIZE;
    }

  } else {  // 不保证数据在内存上的连续

    // 将当前头部数据块的剩余空间利用起来
    block->data = block->payload;
    block->data_size += resv_size;
    buf->total_size += resv_size;
    size -= resv_size;
  }

  // 为数据包分配数据块列表
  net_err_t err = pktblock_list_alloc(buf, size, PKTBUF_LIST_INSERT_HEAD);
  if (err != NET_ERR_OK) {  // 分配失败
    dbg_error(DBG_PKTBUF, "pktbuf add header failed, no buffer for (size %d).",
              size);
    return NET_ERR_NOSRC;
  }

  display_check_buf(buf);  // 检查数据包是否正确
  return NET_ERR_OK;
}

/**
 * @brief 移除数据包头部的数据块
 *
 * @param buf
 * @param size
 * @return net_err_t
 */
net_err_t pktbuf_remove_header(pktbuf_t *buf, int size) {
  // 获取数据包的第一个数据块
  pktblk_t *block = pktbuf_blk_first(buf);

  while (size && block) {
    pktblk_t *next_blk = pktbuf_blk_next(block);

    if (size < block->data_size) {  // 当前数块内容不需要全部移除
      block->data += size;
      block->data_size -= size;
      buf->total_size -= size;
      break;
    }

    // 当前数据块内容全部移除
    int curr_size = block->data_size;
    nlist_remove_first(&(buf->blk_list));  // 从数据块列表中移除数据块
    pktblock_free(block);                  // 释放数据块
    buf->total_size -= curr_size;          // 更新数据包总大小
    size -= curr_size;                     // 更新待移除数据大小

    block = next_blk;
  }

  display_check_buf(buf);  // 检查数据包是否正确

  return NET_ERR_OK;
}

/**
 * @brief 调整数据包大小, 从尾部开始调整
 *
 * @param buf
 * @param to_size
 * @return net_err_t
 */
net_err_t pktbuf_resize(pktbuf_t *buf, int to_size) {
  if (to_size == buf->total_size) {
    return NET_ERR_OK;
  }

  if (buf->total_size == 0) {  // 数据包为空,直接分配数据块列表
    net_err_t err = pktblock_list_alloc(buf, to_size, PKTBUF_LIST_INSERT_TAIL);
    if (err != NET_ERR_OK) {
      dbg_error(DBG_PKTBUF, "pktbuf resize failed, no buffer for (size %d).",
                to_size);
      return NET_ERR_NOSRC;
    }
  } else if (to_size == 0) {  // 清空数据包
    pktblock_list_free(&(buf->blk_list));
    buf->total_size = 0;
    nlist_init(&(buf->blk_list));

  } else if (to_size > buf->total_size) {  // 目标数据大小大于当前数据包大小,
                                           // 需要增加数据包大小
    // 获取尾部数据块
    pktblk_t *tail_blk = pktbuf_blk_last(buf);

    // 计算增加的大小
    int inc_size = to_size - buf->total_size;
    // 计算尾部数据块的剩余空闲空间
    int remain_size = blk_tail_free_size(tail_blk);
    // 调整数据包大小
    if (remain_size >= inc_size) {  // 尾部数据块剩余空间足够
      tail_blk->data_size += inc_size;
      buf->total_size += inc_size;
    } else {  // 尾部数据块剩余空间不够
      // 为数据包分配数据块列表
      net_err_t err = pktblock_list_alloc(buf, (inc_size - remain_size),
                                          PKTBUF_LIST_INSERT_TAIL);
      if (err != NET_ERR_OK) {  // 分配失败
        dbg_error(DBG_PKTBUF, "pktbuf resize failed, no buffer for (size %d).",
                  to_size);
        return NET_ERR_NOSRC;
      }

      // 数据列表分配成功，再将之前尾部数据块的剩余空间利用起来
      tail_blk->data_size += remain_size;
      buf->total_size += remain_size;
    }
  } else {  // 目标数据大小小于当前数据大小，需要减小数据包大小
    int dest_total_size = 0;  //  记录目标大小
    pktblk_t *tail_blk = (pktblk_t *)0;

    // 从头部开始遍历数据块, 记录总的数据大小, 当总的数据大小大于等于目标大小则停止
    for (tail_blk = pktbuf_blk_first(buf); tail_blk; tail_blk = pktbuf_blk_next(tail_blk)) {
      dest_total_size += tail_blk->data_size;
      if (dest_total_size >= to_size) {
        break;
      }
    }
    
    if (tail_blk == (pktblk_t *)0) {
      dbg_error(DBG_PKTBUF, "pktbuf resize failed, decrease size error(size %d to %d).",
               buf->total_size, to_size);
      return NET_ERR_SIZE;
    }

    // 目标大小已经达到, 移除多余的数据块
    pktblk_t *curr_blk = pktbuf_blk_next(tail_blk);
    while (curr_blk) {
      pktblk_t *next_blk = pktbuf_blk_next(curr_blk);
      nlist_remove(&(buf->blk_list), &(curr_blk->node));
      pktblock_free(curr_blk);
      curr_blk = next_blk;
    }

    // 调整尾部数据块需要移除的数据大小
    int remove_size = dest_total_size - to_size;
    tail_blk->data_size -= remove_size;
    buf->total_size = to_size;
  }

  // 检查数据包大小是否调整正确
  if (buf->total_size != to_size) {
    dbg_error(DBG_PKTBUF, "pktbuf resize failed, size error(%d != %d).",
              buf->total_size, to_size);
    return NET_ERR_SIZE;
  }

  display_check_buf(buf);  // 检查数据包是否正确

  return NET_ERR_OK;
}

/**
 * @brief 合并两个数据包, 将src的数据块列表合并到dest的数据块列表中
 * 并将src数据包释放
 * 
 * @param dest 
 * @param src 
 * @return net_err_t 
 */
net_err_t pktbuf_join(pktbuf_t * dest, pktbuf_t * src) {
  if (dest == (pktbuf_t *)0 || src == (pktbuf_t *)0) {
    dbg_error(DBG_PKTBUF, "pktbuf join failed, invalid pktbuf.");
    return NET_ERR_PARAM;
  }

  // 将src的数据块列表添加到dest的数据块列表中
  nlist_join(&(dest->blk_list), &(src->blk_list));
  // 更新数据包的总大小
  dest->total_size += src->total_size;

  // 释放src数据包
  pktbuf_free(src);

  display_check_buf(dest);  // 检查数据包是否正确

  return NET_ERR_OK;
}