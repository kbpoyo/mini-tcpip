/**
 * @file mblock.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief
 * @version 0.1
 * @date 2024-05-12
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "mblock.h"

#include "dbg.h"

/**
 * @brief 初始化内存块管理结构
 *
 * @param mblock 内存块管理结构
 * @param mem_start 管理的内存块的起始地址
 * @param blk_size 内存块的大小
 * @param blk_cnt  内存块的数量
 * @param locker_type 使用的锁类型
 * @return net_err_t
 */
net_err_t mblock_init(mblock_t *mblock, void *mem_start, size_t blk_size,
                      size_t blk_cnt, nlocker_type_t locker_type) {
  mblock->start = mem_start;
  uint8_t *buf = (uint8_t *)mem_start;

  nlist_init(&mblock->free_list);

  // 将每一个内存块的起始部分当作一个节点插入到空闲链表中
  for (int i = 0; i < blk_cnt; i++) {
    nlist_insert_last(&mblock->free_list, (nlist_node_t *)&buf[i * blk_size]);
  }

  nlocker_init(&mblock->locker, locker_type);

  // 当资源需要锁保护时，初始化内存块资源的信号量
  if (locker_type != NLOCKER_NONE) {
    mblock->alloc_sem = sys_sem_create(blk_cnt);
    if (mblock->alloc_sem == SYS_SEM_INVALID) {
      dbg_error(DBG_MBLOCK, "create alloc_sem failed.");
      nlocker_destroy(&mblock->locker);
      return NET_ERR_SYS;
    }
  }
}

/**
 * @brief 从内存块管理结构中分配一个内存块
 *
 * @param mblock
 * @param ms 等待时间
 * @return void*
 */
void *mblock_alloc(mblock_t *mblock, int ms) {
  // 若ms < 0 或者内存管理块结构不需要锁，则内存块的分配不需要等待
  if (ms < 0 || mblock->locker.type == NLOCKER_NONE) {
    nlocker_lock(&mblock->locker);  // 在ms < 0时且有锁的情况下有效

    int count = nlist_count(&mblock->free_list);
    if (count == 0) {  // 没有空闲内存块直接返回
      nlocker_unlock(&mblock->locker);
      return (void *)0;
    } else {
      // 将内存卡从空闲链表中取出并返回给调用者使用
      nlist_node_t *block = nlist_remove_first(&mblock->free_list);
      nlocker_unlock(&mblock->locker);

      return block;
    }

  } else {  // 需要等待

    // 等待资源可用
    if (sys_sem_wait(mblock->alloc_sem, ms) < 0) {  // 等待超时或出错
      return (void *)0;
    } else {  // 获取资源成功，进行内存块的分配
      nlocker_lock(&mblock->locker);
      nlist_node_t *block = nlist_remove_first(&mblock->free_list);
      nlocker_unlock(&mblock->locker);
      return block;
    }
  }
}

/**
 * @brief 获取空闲内存块的数量
 *
 * @param mblock
 * @return int
 */
int mblock_free_cnt(mblock_t *mblock) {
  nlocker_lock(&mblock->locker);

  int cnt = nlist_count(&mblock->free_list);

  nlocker_unlock(&mblock->locker);

  return cnt;
}
