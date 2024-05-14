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
        if (curr->data < curr->payload || curr->data >= curr->payload + PKTBUF_BLK_SIZE) {
            dbg_error(DBG_PKTBUF, "block data error, %p not in [%p, %p)", curr->data, curr->payload, curr->payload + PKTBUF_BLK_SIZE);
        }

        int pre_size = (int)(curr->data - curr->payload);
        plat_printf("pre: %d B,\t", pre_size);

        int used_size = curr->data_size;
        plat_printf("used: %d B,\t", used_size);

        int free_size = blk_tail_free_size(curr);
        plat_printf("free: %d B\n", free_size);

        int blk_total_size = pre_size + used_size + free_size;
        if (blk_total_size != PKTBUF_BLK_SIZE) {
            dbg_error(DBG_PKTBUF, "block total size error, %d != %d", blk_total_size, PKTBUF_BLK_SIZE);
        }

        buf_total_size += used_size;
    }

    // 检查数据包的总大小是否正确
    if (buf_total_size != pktbuf->total_size) {
        dbg_error(DBG_PKTBUF, "buf total size error, %d != %d", buf_total_size, pktbuf->total_size);
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
 * @brief 将数据库列表中的数据块释放到数据块池中
 *
 * @param blk_list
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
    mblock_free(&pktblk_list, pktblk);

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
static net_err_t pktblock_list_alloc(pktbuf_t* pktbuf, int size,
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
    nlist_merge(&tmp_list, &(pktbuf->blk_list));
    nlist_move(&(pktbuf->blk_list), &tmp_list);
  } else {  // 使用尾插法
    nlist_merge(&(pktbuf->blk_list), &tmp_list);
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
 * @brief
 *
 * @param pktbuf
 */
void pktbuf_free(pktbuf_t *pktbuf) {}