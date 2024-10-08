/**
 * @file fixq.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 定长队列模块
 * @version 0.1
 * @date 2024-05-12
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "fixq.h"

#include "dbg.h"

/**
 * @brief 定长队列初始化
 *
 * @param q 定长队列
 * @param buf 定长队列缓冲区
 * @param size 定长队列大小
 * @param locker_type 使用的锁类型
 * @return net_err_t
 */
net_err_t fixq_init(fixq_t *q, void **buf, int size,
                    nlocker_type_t locker_type) {
  q->size = size;
  q->in = q->out = q->cnt = 0;
  q->buf = buf;
  q->get_sem = q->put_sem = SYS_SEM_INVALID;

  // 初始化锁
  net_err_t err = nlocker_init(&q->locker, locker_type);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_FIXQ, "locker init failed.");
    return err;
  }

  // 初始化get信号量(队列中可获取的资源数量)
  q->get_sem = sys_sem_create(0);
  if (q->get_sem == SYS_SEM_INVALID) {
    dbg_error(DBG_FIXQ, "get_sem init failed.");
    goto init_failed;
  }

  // 初始化put信号量(队列中可放入的空槽位数量)
  q->put_sem = sys_sem_create(size);
  if (q->put_sem == SYS_SEM_INVALID) {
    dbg_error(DBG_FIXQ, "put_sem init failed.");
    goto init_failed;
  }

  return NET_ERR_OK;

init_failed:
  nlocker_destroy(&(q->locker));

  if (q->get_sem != SYS_SEM_INVALID) {
    sys_sem_free(q->get_sem);
  }
  if (q->put_sem != SYS_SEM_INVALID) {
    sys_sem_free(q->put_sem);
  }
  return NET_ERR_SYS;
}

/**
 * @brief 向队列中放入数据
 *
 * @param q
 * @param msg
 * @param tmo_ms 等待时间
 * @return net_err_t
 */
net_err_t fixq_put(fixq_t *q, void *msg, int tmo_ms) {
  nlocker_lock(&q->locker);

  if (tmo_ms < 0 && q->cnt >= q->size) {  // 队列已满，且不进行等待
    nlocker_unlock(&q->locker);
    return NET_ERR_FULL;
  }
  nlocker_unlock(&q->locker);

  // 等待队列中有空闲位置
  if (sys_sem_wait(q->put_sem, tmo_ms) < 0) {
    return NET_ERR_TIMEOUT;
  }

  // 向队列中发送数据
  nlocker_lock(&q->locker);
  q->buf[(q->in)++] = msg;
  q->in %= q->size;  // 循环队列
  q->cnt++;
  nlocker_unlock(&q->locker);

  // 已放入一个资源，增加get信号量值
  sys_sem_notify(q->get_sem);

  return NET_ERR_OK;
}

/**
 * @brief 从队列中取出数据
 *
 * @param q
 * @param ms
 * @return void*
 */
void *fixq_get(fixq_t *q, int tmo_ms) {
  nlocker_lock(&q->locker);

  if (q->cnt == 0 && tmo_ms < 0) {  // 队列为空，且不进行等待
    nlocker_unlock(&q->locker);
    return (void *)0;
  }
  nlocker_unlock(&q->locker);

  // 等待队列中有数据
  if (sys_sem_wait(q->get_sem, tmo_ms) < 0) {
    return (void *)0;
  }

  // 从队列中取出数据
  nlocker_lock(&q->locker);
  void *msg = q->buf[(q->out)++];
  q->out %= q->size;
  q->cnt--;
  nlocker_unlock(&q->locker);

  // 已拿走一个资源，空槽数增加，增加put信号量值
  sys_sem_notify(q->put_sem);

  return msg;
}

/**
 * @brief 获取队列中的数据数量
 *
 * @param q
 */
int fixq_count(fixq_t *q) {
  nlocker_lock(&q->locker);
  int cnt = q->cnt;
  nlocker_unlock(&q->locker);

  return cnt;
}

/**
 * @brief 销毁队列对象
 *
 * @param q
 */
void fixq_destroy(fixq_t *q) {
  nlocker_destroy(&q->locker);
  sys_sem_free(q->get_sem);
  sys_sem_free(q->put_sem);
}