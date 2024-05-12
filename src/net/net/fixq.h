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
    int size;
    int in, out, cnt;

    void **buf;

    nlocker_t locker;
    sys_sem_t recv_sem; // 接收线程信号量
    sys_sem_t send_sem; // 发送线程信号量

} fixq_t;


net_err_t fixq_init(fixq_t *q, void **buf, int size, nlocker_type_t locker_type);

net_err_t fixq_send(fixq_t *q, void *msg, int ms);

#endif  // FIXQ_H