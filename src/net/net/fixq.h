/**
 * @file fixq.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 消息队列模块
 * @version 0.1
 * @date 2024-05-12
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef FIXQ_H
#define FIXQ_H

#include "nlocker.h"
#include "net_sys.h"

typedef struct _fixq_t {
    int size; // 队列大小
    int in, out, cnt;   // in:入队索引 out:出队索引 cnt:队列中元素个数

    void **buf; // 消息队列缓冲区

    nlocker_t locker; // 锁
    sys_sem_t recv_sem; // 接收线程信号量
    sys_sem_t send_sem; // 发送线程信号量

} fixq_t;


net_err_t fixq_init(fixq_t *q, void **buf, int size, nlocker_type_t locker_type);
net_err_t fixq_send(fixq_t *q, void *msg, int ms);
void *fixq_recv(fixq_t *q, int tmo_ms);
void fixq_destroy(fixq_t *q);
int fixq_count(fixq_t *q);

#endif  // FIXQ_H