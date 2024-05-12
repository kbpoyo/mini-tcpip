/**
 * @file mblock.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief
 * @version 0.1
 * @date 2024-05-12
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef MBLOCK_H
#define MBLOCK_H

#include "nlist.h"
#include "nlocker.h"

/**
 * @brief 内存块管理结构
 */
typedef struct _mblock_t {
  nlist_t free_list;    //空闲链表
  void *start;  //记录内存块的起始地址

  nlocker_t locker;  //内存块资源的锁
  sys_sem_t alloc_sem;  //内存卡资源的信号量

} mblock_t;


net_err_t mblock_init(mblock_t *mblock, void *mem_start, size_t blk_size, size_t blk_cnt, nlocker_type_t locker_type);

void *mblock_alloc(mblock_t *mblock, int ms);

int mblock_free_cnt(mblock_t *mblock);

#endif  // MBLOCK_H