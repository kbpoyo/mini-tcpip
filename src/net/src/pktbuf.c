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
#include "tools.h"
#include "dbg.h"
#include "mblock.h"

static pktblk_t pktblk_pool[PKTBUF_BLK_CNT];  // 数据块池
static mblock_t pktblk_list;  // 数据块链表(管理数据块池)

static pktbuf_t pktbuf_pool[PKTBUF_BUF_CNT];  // 数据包池
static mblock_t pktbuf_list;  // 数据包链表(管理数据包池)

static nlocker_t pkt_locker;  // 数据包模块锁

static void pktbuf_pos_move_forward(pktbuf_t *buf, int size);

static inline void pktbuf_update_pos(pktbuf_t *buf) {
  dbg_assert(buf->pos >= 0, "pos error.");

  // 当前数据包访问位置已更新到无效位置
  if (buf->pos >= buf->total_size) {
    buf->pos = buf->total_size;
    buf->curr_blk = (pktblk_t *)0;
    buf->curr_pos = (uint8_t *)0;
    return;
  }

  // 当前数据包访问位置已更新到有效位置
  if (buf->curr_blk == (pktblk_t *)0 && buf->curr_pos == (uint8_t *)0) {
  
    int offset = buf->pos;
    pktbuf_acc_reset(buf);  // 重置数据包的访问位置到数据包的起始位置
    pktbuf_pos_move_forward(buf, offset);  // 移动数据包的访问位置到指定位置
  }
}

/**
 * @brief 获取数据包当前访问数据块的剩余待读取字节数
 *
 * @param buf
 * @return int
 */
static inline int pktbuf_currblk_remain_size(const pktbuf_t *buf) {
  pktblk_t *blk = buf->curr_blk;
  if (blk == (pktblk_t *)0) {
    return 0;
  }

  return (int)(buf->curr_blk->data_size -
               (buf->curr_pos - buf->curr_blk->data));
}

/**
 * @brief 获取数据块的尾部剩余空闲空间大小
 *
 * @param blk
 * @return int
 */
static inline int pktblk_tail_free_size(pktblk_t *blk) {
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

    int free_size = pktblk_tail_free_size(curr);
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
net_err_t pktbuf_module_init(void) {
  dbg_info(DBG_PKTBUF, "init pktbuf module....");

  // TODO: 当前只使用一个互斥锁来保护数据包模块(包括数据包池和数据块池), 后续可以考虑使用更细粒度的锁
  net_err_t err = nlocker_init(&pkt_locker, NLOCKER_THREAD);
  Net_Err_Check(err);

  err = mblock_init(&pktblk_list, pktblk_pool, sizeof(pktblk_t), PKTBUF_BLK_CNT,
                    NLOCKER_NONE);
  Net_Err_Check(err);

  err = mblock_init(&pktbuf_list, pktbuf_pool, sizeof(pktbuf_t), PKTBUF_BUF_CNT,
                    NLOCKER_NONE);
  Net_Err_Check(err);

  dbg_info(DBG_PKTBUF, "init pktbuf module ok.");
  return NET_ERR_OK;
}

/**
 * @brief 从数据块池中分配一个数据块
 *
 * @return pktblk_t*
 */
static pktblk_t *pktblock_alloc(void) {
  nlocker_lock(&pkt_locker);  // 加锁
  pktblk_t *pktblk = (pktblk_t *)mblock_alloc(&pktblk_list, -1);
  nlocker_unlock(&pkt_locker);  // 解锁

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
  nlocker_lock(&pkt_locker);  // 加锁
  mblock_free(&pktblk_list, pktblk);
  nlocker_unlock(&pkt_locker);  // 解锁
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
 * @brief 从数据块池中分配一个数据块列表,并将其插入数据包中
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
  nlocker_lock(&pkt_locker);  // 加锁
  // 分配一个数据包
  pktbuf_t *pktbuf = (pktbuf_t *)mblock_alloc(&pktbuf_list, -1);
  nlocker_unlock(&pkt_locker);  // 解锁

  if (pktbuf == (pktbuf_t *)0) {
    dbg_error(DBG_PKTBUF, "pktbuf alloc failed, no buffer.");
    return (pktbuf_t *)0;
  }

  // 初始化数据包
  pktbuf->total_size = 0;
  pktbuf->ref_cnt = 1;
  nlist_init(&(pktbuf->blk_list));
  nlist_node_init(&(pktbuf->node));

  if (size > 0) {
    // 为数据包分配数据块列表
    net_err_t err = pktblock_list_alloc(pktbuf, size, PKTBUF_LIST_INSERT_HEAD);
    if (err != NET_ERR_OK) {      // 分配失败
      nlocker_lock(&pkt_locker);  // 加锁
      // 释放数据包
      mblock_free(&pktbuf_list, pktbuf);

      nlocker_unlock(&pkt_locker);  // 解锁
      return (pktbuf_t *)0;
    }
  }

  // 初始化数据包的访问位置
  pktbuf_acc_reset(pktbuf);

  display_check_buf(pktbuf);

  return pktbuf;
}

/**
 * @brief 释放数据包对象
 *
 * @param pktbuf
 */
void pktbuf_free(pktbuf_t *buf) {
  pktbuf_check_buf(buf);

  // 将包的ref_cnt也保护起来，避免包递交给其它线程后，当前线程和其它线程同时对其进行释放

  nlocker_lock(&pkt_locker);    // 加锁
  if (--(buf->ref_cnt) == 0) {  // 引用计数为0,释放数据包
    // 释放数据块列表
    pktblock_list_free(&(buf->blk_list));
    // 释放数据包
    mblock_free(&pktbuf_list, buf);
  }
  nlocker_unlock(&pkt_locker);  // 解锁
}

/**
 * @brief 在数据包的头部增加size个字节的数据
 *
 * @param buf
 * @param size
 * @param is_cont
 * 若头部数据块剩余空间不够,是否将数据内容全部放入新的数据块中，以保证数据在内存上的连续
 * @return net_err_t
 */
net_err_t pktbuf_header_add(pktbuf_t *buf, int size, int is_cont) {
  pktbuf_check_buf(buf);

  // 如果当前数据包为空,直接分配数据块列表
  if (buf->total_size == 0) {
    net_err_t err = pktblock_list_alloc(buf, size, PKTBUF_LIST_INSERT_HEAD);
    if (err != NET_ERR_OK) {
      dbg_error(DBG_PKTBUF,
                "pktbuf add header failed, no buffer for (size %d).", size);
      return NET_ERR_NOSRC;
    }

    return NET_ERR_OK;
  }

  // 将当前数据包的第一个数据块剩余空闲空间利用起来
  pktblk_t *block = pktbuf_blk_first(buf);
  // 计算当前数据块头部的剩余空闲空间
  int resv_size = (int)(block->data - block->payload);
  if (size <= resv_size) {  // 头部剩余空间足够拓展
    block->data -= size;
    block->data_size += size;
    buf->total_size += size;

    display_check_buf(buf);  // 检查数据包是否正确
    return NET_ERR_OK;
  }

  // 头部数据块的剩余空闲空间不够
  // 再为数据包分配数据块列表
  if (is_cont == PKTBUF_ADD_HEADER_CONT) {  // 保证数据在内存上的连续
    // 待分配内存大于数据块有效载荷大小，无法实现在内存上的连续
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
 * @brief 从数据包头部移除size个字节的数据
 *
 * @param buf
 * @param size
 * @return net_err_t
 */
net_err_t pktbuf_header_remove(pktbuf_t *buf, int size) {
  pktbuf_check_buf(buf);

  // 获取数据包的第一个数据块
  pktblk_t *block = pktbuf_blk_first(buf);

  // 从第一个数据块开始移除数据
  int remove_size = size;
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

  // 判断数据包访问位置是否需要更新，若当前访问位置在移除的数据块中，则更新访问位置
  buf->pos -= remove_size;
  if (buf->pos < 0) {
    pktbuf_acc_reset(buf);
  }

  display_check_buf(buf);  // 检查数据包是否正确

  return NET_ERR_OK;
}

/**
 * @brief 调整数据包大小为to_size, 且从尾部开始调整
 *
 * @param buf
 * @param to_size
 * @return net_err_t
 */
net_err_t pktbuf_resize(pktbuf_t *buf, int to_size) {
  pktbuf_check_buf(buf);

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
    int remain_size = pktblk_tail_free_size(tail_blk);
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
    pktblk_t *curr_blk = (pktblk_t *)0;

    // 从头部开始遍历数据块, 记录总的数据大小,
    // 当总的数据大小大于等于目标大小则停止
    for (curr_blk = pktbuf_blk_first(buf); curr_blk;
         curr_blk = pktbuf_blk_next(curr_blk)) {
      dest_total_size += curr_blk->data_size;
      if (dest_total_size >= to_size) {
        break;
      }
    }

    if (curr_blk == (pktblk_t *)0) {  // 目标大小达不到，返回错误值
      dbg_error(DBG_PKTBUF,
                "pktbuf resize failed, decrease size error(size %d to %d).",
                buf->total_size, to_size);
      return NET_ERR_SIZE;
    }

    // 目标大小已经达到, 移除多余的数据块
    pktblk_t *tail_blk = curr_blk;  // 记录新的尾部数据块
    curr_blk = pktbuf_blk_next(curr_blk);
    while (curr_blk) {
      pktblk_t *next_blk = pktbuf_blk_next(curr_blk);

      nlist_remove(&(buf->blk_list), &(curr_blk->node));
      buf->total_size -= curr_blk->data_size;

      pktblock_free(curr_blk);
      curr_blk = next_blk;
    }

    // 调整尾部数据块需要移除的数据大小
    int remove_size = dest_total_size - to_size;
    tail_blk->data_size -= remove_size;
    buf->total_size -= remove_size;
  }

  // 检查数据包大小是否调整正确
  if (buf->total_size != to_size) {
    dbg_error(DBG_PKTBUF, "pktbuf resize failed, size error(%d != %d).",
              buf->total_size, to_size);
    return NET_ERR_SIZE;
  }

  // 更新数据包的访问位置
  pktbuf_update_pos(buf);

  display_check_buf(buf);  // 检查数据包是否正确

  return NET_ERR_OK;
}

/**
 * @brief 合并两个数据包, 将src的数据块列表合并到dest的数据块列表中
 * 并将src数据包释放
 *
 * @param dest 合并后的目标数据包
 * @param src 被合并的数据包，合并后将被释放
 * @return net_err_t
 */
net_err_t pktbuf_join(pktbuf_t *dest, pktbuf_t *src) {
  pktbuf_check_buf(dest);
  pktbuf_check_buf(src);

  // 将src的数据块列表添加到dest的数据块列表中
  nlist_join(&(dest->blk_list), &(src->blk_list));
  // 更新数据包的总大小
  dest->total_size += src->total_size;

  // 释放src数据包
  pktbuf_free(src); //!!! 释放数据包

  // 更新数据包的访问位置
  pktbuf_update_pos(dest);

  display_check_buf(dest);  // 检查数据包是否正确

  return NET_ERR_OK;
}

/**
 * @brief 设置数据包前size个字节的数据在内存上连续，
 * 即将数据包前size个字节调整到头部数据块中
 *
 * @param buf
 * @param size
 * @return net_err_t
 */
net_err_t pktbuf_set_cont(pktbuf_t *buf, int size) {
  pktbuf_check_buf(buf);

  if (size > buf->total_size ||
      size > PKTBUF_BLK_SIZE) {  // size大于数据包总大小或数据块有效载荷大小
    dbg_error(DBG_PKTBUF, "pktbuf set cont failed, size too big (%d > %d).",
              size, buf->total_size);
    return NET_ERR_SIZE;
  }

  pktblk_t *first_blk = pktbuf_blk_first(buf);
  if (size <= first_blk->data_size) {  // 数据包起始size个字节在第一个数据块中
    display_check_buf(buf);
    return NET_ERR_OK;
  }

  // 将第一个数据块的数据调整到数据块有效载荷的起始位置
  uint8_t *dest = first_blk->payload;
  if (first_blk->data != first_blk->payload) {
    for (int i = 0; i < first_blk->data_size; i++) {
      dest[i] = first_blk->data[i];
    }
    first_blk->data = first_blk->payload;
  }
  dest += first_blk->data_size;  // 调整dest到空闲区域的起始位置

  // 计算还需要调整多少字节
  int remain_size = size - first_blk->data_size;
  pktblk_t *curr_blk = pktbuf_blk_next(first_blk);
  while (remain_size &&
         curr_blk) {  // 在后续块调整remain_size字节到第一个数据块中

    // 计算当前块中待调整的字节数
    int curr_size =
        (curr_blk->data_size > remain_size) ? remain_size : curr_blk->data_size;

    // 将当前块待调整的数据调整到第一个数据块中
    plat_memcpy(dest, curr_blk->data, curr_size);

    // 更新dest，remain_size
    dest += curr_size;
    remain_size -= curr_size;
    // 更新头部数据块与当前数据块
    first_blk->data_size += curr_size;
    curr_blk->data += curr_size;
    curr_blk->data_size -= curr_size;

    // 当前数据块内容已被全部移到前一个数据块，需要将当前数据块移除
    if (curr_blk->data_size == 0) {
      pktblk_t *next_blk = pktbuf_blk_next(curr_blk);

      nlist_remove(&(buf->blk_list), &(curr_blk->node));
      pktblock_free(curr_blk);

      curr_blk = next_blk;
    }
  }

  display_check_buf(buf);
  return NET_ERR_OK;
}

/**
 * @brief 重置数据包的访问位置到数据包的起始位置
 *
 * @param buf
 */
void pktbuf_acc_reset(pktbuf_t *buf) {
  pktbuf_check_buf(buf);

  buf->pos = 0;
  buf->curr_blk = pktbuf_blk_first(buf);
  buf->curr_pos = buf->curr_blk ? buf->curr_blk->data : (uint8_t *)0;
}

/**
 * @brief 将数据包待访问位置向前移动size个字节
 *
 * @param buf
 * @param size
 */
static void pktbuf_pos_move_forward(pktbuf_t *buf, int size) {
  while (size && buf->curr_blk) {
    int currblk_remain_size = pktbuf_currblk_remain_size(buf);
    int curr_move_size =
        (size > currblk_remain_size) ? currblk_remain_size : size;

    // 更新当前数据块的访问位置
    buf->pos += curr_move_size;
    buf->curr_pos += curr_move_size;
    size -= curr_move_size;

    // 当前数据块已访问完，更新当前数据块和当前位置到下一个数据块
    if (buf->curr_pos >=
        (buf->curr_blk->data +
         buf->curr_blk->data_size)) {  // 实际情况中应该只有等于
      buf->curr_blk = pktbuf_blk_next(buf->curr_blk);
      buf->curr_pos = buf->curr_blk ? buf->curr_blk->data : (uint8_t *)0;
    }
  }

  if (size) {
    dbg_error(DBG_PKTBUF, "pktbuf pos move failed, move size error(%d).", size);
  }
}

/**
 * @brief 向数据包中写入数据
 *
 * @param buf 数据包
 * @param src 待写入数据的缓冲区
 * @param size 待写入数据的大小
 * @return net_err_t
 */
net_err_t pktbuf_write(pktbuf_t *buf, uint8_t *src, int size) {
  pktbuf_check_buf(buf);

  if (src == (uint8_t *)0 || size < 0) {
    dbg_error(DBG_PKTBUF, "pktbuf write failed, invalid param.");
    return NET_ERR_PARAM;
  }

  // 检查数据包是否有足够的空间
  int remain_size = pktbuf_remain_size(buf);
  if (remain_size < size) {
    dbg_error(DBG_PKTBUF, "pktbuf write failed, no enough buffer(%d < %d).",
              remain_size, size);
    return NET_ERR_SIZE;
  }

  // 写入数据
  while (size) {
    int curr_size = pktbuf_currblk_remain_size(buf);

    curr_size = (curr_size > size) ? size : curr_size;
    plat_memcpy(buf->curr_pos, src, curr_size);
    pktbuf_pos_move_forward(buf, curr_size);

    src += curr_size;
    size -= curr_size;
  }

  return NET_ERR_OK;
}

/**
 * @brief 读取数据包中的数据
 *
 * @param buf 数据包
 * @param dest 读取数据的缓冲区
 * @param size 读取数据的大小
 * @return net_err_t
 */
net_err_t pktbuf_read(pktbuf_t *buf, uint8_t *dest, int size) {
  pktbuf_check_buf(buf);

  if (dest == (uint8_t *)0 || size < 0) {
    dbg_error(DBG_PKTBUF, "pktbuf read failed, invalid param.");
    return NET_ERR_PARAM;
  }

  // 检查数据包是否有足够的空间
  int remain_size = pktbuf_remain_size(buf);
  if (remain_size < size) {
    dbg_error(DBG_PKTBUF, "pktbuf read failed, no enough buffer(%d < %d).",
              remain_size, size);
    return NET_ERR_SIZE;
  }

  // 读取数据
  while (size) {
    int curr_size = pktbuf_currblk_remain_size(buf);

    curr_size = (curr_size > size) ? size : curr_size;
    plat_memcpy(dest, buf->curr_pos, curr_size);
    pktbuf_pos_move_forward(buf, curr_size);

    dest += curr_size;
    size -= curr_size;
  }
  return NET_ERR_OK;
}

/**
 * @brief 将数据包当前访问位置移动到起始地址偏移offset个字节处
 *
 * @param buf
 * @param offset
 * @return net_err_t
 */
net_err_t pktbuf_seek(pktbuf_t *buf, int offset) {
  pktbuf_check_buf(buf);

  if (buf->pos == offset) {  // 当前位置已经在目标位置
    return NET_ERR_OK;
  }

  if (offset < 0 || offset > buf->total_size) {  // 目标位置不合法
    dbg_error(DBG_PKTBUF, "pktbuf seek failed, invalid offset(%d).", offset);
    return NET_ERR_PARAM;
  }

  // 计算目标位置相对于当前位置的偏移
  int move_size = 0;
  if (offset > buf->pos) {  // 目标位置在当前位置之后
    move_size = offset - buf->pos;
  } else {                  // 目标位置在当前位置之前
    pktbuf_acc_reset(buf);  // 重置访问位置
    move_size = offset;
  }

  // 移动当前访问位置
  pktbuf_pos_move_forward(buf, move_size);

  dbg_assert(buf->pos == offset, "pktbuf seek failed.");

  return NET_ERR_OK;
}

/**
 * @brief 从src数据包当前访问位置开始复制size个字节到dest数据包中(从dest当前访问位置开始)
 * 该方法会修改buf的当前访问位置pos
 *
 * @param dest
 * @param src
 * @param size
 * @return net_err_t
 */
net_err_t pktbuf_copy(pktbuf_t *dest, pktbuf_t *src, int size) {
  pktbuf_check_buf(dest);
  pktbuf_check_buf(src);

  if (size < 0) {
    dbg_error(DBG_PKTBUF, "pktbuf copy failed, invalid param.");
    return NET_ERR_PARAM;
  }

  if (pktbuf_remain_size(src) < size || pktbuf_remain_size(dest) < size) {
    dbg_error(DBG_PKTBUF, "pktbuf copy failed, dest or src size too small.");
    return NET_ERR_PARAM;
  }

  // 进行数据拷贝
  while (size) {
    int dest_remain_size = pktbuf_currblk_remain_size(dest);
    int src_remain_size = pktbuf_currblk_remain_size(src);
    int curr_copy_size = (dest_remain_size > src_remain_size)
                             ? src_remain_size
                             : dest_remain_size;
    curr_copy_size = (curr_copy_size > size) ? size : curr_copy_size;

    // 从src数据包中拷贝curr_copy_size个字节到dest数据包中
    plat_memcpy(dest->curr_pos, src->curr_pos, curr_copy_size);
    pktbuf_pos_move_forward(dest, curr_copy_size);
    pktbuf_pos_move_forward(src, curr_copy_size);

    // 更新剩余拷贝字节数
    size -= curr_copy_size;
  }

  return NET_ERR_OK;
}

/**
 * @brief 用值data填充数据包size个字节
 *
 * @param buf
 * @param data
 * @param size
 * @return net_err_t
 */
net_err_t pktbuf_fill(pktbuf_t *buf, uint8_t data, int size) {
  pktbuf_check_buf(buf);

  if (size < 0) {
    dbg_error(DBG_PKTBUF, "pktbuf write failed, invalid param.");
    return NET_ERR_PARAM;
  }

  // 检查数据包是否有足够的空间
  int remain_size = pktbuf_remain_size(buf);
  if (remain_size < size) {
    dbg_error(DBG_PKTBUF, "pktbuf write failed, no enough buffer(%d < %d).",
              remain_size, size);
    return NET_ERR_SIZE;
  }

  // 写入数据
  while (size) {
    // 计算当前数据块的剩余空间
    int curr_fill_size = pktbuf_currblk_remain_size(buf);

    // 计算能在当前数据块中填充的字节数
    curr_fill_size = (curr_fill_size > size) ? size : curr_fill_size;
    plat_memset(buf->curr_pos, data, curr_fill_size);

    // 更新数据包的访问位置
    pktbuf_pos_move_forward(buf, curr_fill_size);

    size -= curr_fill_size;
  }

  return NET_ERR_OK;
}

/**
 * @brief 增加数据包的引用计数
 *
 * @param buf
 */
pktbuf_t *pktbuf_inc_ref(pktbuf_t *buf) {
  // 数据包的引用计数可能被多个线程操作，需要对其进行加锁保护

  nlocker_lock(&pkt_locker);  // 加锁
  buf->ref_cnt++;
  nlocker_unlock(&pkt_locker);  // 解锁

  return buf;
}

/**
 * @brief 计算数据包从当前访问位置buf.pos开始的size个字节的校验和
 * 
 * @param buf 参与计算校验和的数据起始地址
 * @param size 参与计算校验和的数据字节数
 * @param pre_sum 起始校验和
 * @param is_take_back 是否对计算结果取反(1:取反 0:不取反)
 * @return uint16_t 
 */
 uint16_t pktbuf_checksum16(pktbuf_t *buf, uint16_t size, uint32_t pre_sum, int is_take_back) {
  pktbuf_check_buf(buf);

  // 获取数据包当前待访问的数据大小
  int remain_size = pktbuf_remain_size(buf);
  if (size < 0 || size > remain_size) {
    dbg_error(DBG_PKTBUF, "size too small or too big.");
    return 0;
  }

  // 计算校验和
  uint32_t sum = pre_sum;
  while (size > 0) {
    // 获取当前数据块的剩余空间
    int blk_remain_size = pktbuf_currblk_remain_size(buf);
    int curr_size = size > blk_remain_size ? blk_remain_size : size;

    // 计算当前数据块中待访问数据的校验和
    sum = tools_checksum16(buf->curr_pos, curr_size, sum, 0);

    // 更新数据包的访问位置
    pktbuf_pos_move_forward(buf, curr_size);

    // 更新剩余待计算数据大小
    size -= curr_size;
  }

  //根据is_take_back的值，判断是否取反
  return is_take_back ? (uint16_t)~sum : (uint16_t)sum;

 }