/**
 * @file fixq.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 消息队列模块
 * @version 0.1
 * @date 2024-05-12
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "fixq.h"

#include "dbg.h"

/**
 * @brief 消息队列初始化
 *
 * @param q
 * @param buf
 * @param size
 * @param locker_type
 * @return net_err_t
 */
net_err_t fixq_init(fixq_t *q, void **buf, int size,
                    nlocker_type_t locker_type) {
  q->size = size;
  q->in = q->out = q->cnt = 0;
  q->buf = buf;
  q->recv_sem = q->send_sem = SYS_SEM_INVALID;

  // 初始化锁
  net_err_t err = nlocker_init(&q->locker, locker_type);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_FIXQ, "locker init failed.");
    return err;
  }

  // 初始化接收信号量
  q->recv_sem = sys_sem_create(0);
  if (q->recv_sem == SYS_SEM_INVALID) {
    dbg_error(DBG_FIXQ, "recv_sem init failed.");
    goto init_failed;
  }

  // 初始化发送信号量
  q->send_sem = sys_sem_create(size);
  if (q->send_sem == SYS_SEM_INVALID) {
    dbg_error(DBG_FIXQ, "send_sem init failed.");
    goto init_failed;
  }

  return NET_ERR_OK;

init_failed:
  nlocker_destroy(&(q->locker));

  if (q->recv_sem != SYS_SEM_INVALID) {
    sys_sem_free(q->recv_sem);
  }
  if (q->send_sem != SYS_SEM_INVALID) {
    sys_sem_free(q->send_sem);
  }
  return NET_ERR_SYS;
}

/**
 * @brief 向消息队列发送消息
 * 
 * @param q 
 * @param msg 
 * @param tmo_ms 等待时间
 * @return net_err_t 
 */
net_err_t fixq_send(fixq_t *q, void *msg, int tmo_ms) {
  nlocker_lock(&q->locker);

  if (tmo_ms < 0 && q->cnt >= q->size) {  //消息队列已满，且不进行等待
    nlocker_unlock(&q->locker);
    return NET_ERR_FIXQ_FULL;
  }
  nlocker_unlock(&q->locker);

  if (sys_sem_wait(q->send_sem, tmo_ms) < 0) {
    return NET_ERR_TIMEOUT;
  }

  nlocker_lock(&q->locker);

  q->buf[(q->in)++] = msg;
  q->in %= q->size; //循环队列
  q->cnt++;

  nlocker_unlock(&q->locker);

  sys_sem_notify(q->recv_sem);

  return NET_ERR_OK;
}