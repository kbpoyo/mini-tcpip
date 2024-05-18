/**
 * @file nlocker.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief
 * @version 0.1
 * @date 2024-05-12
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef NLOCKER_H
#define NLOCKER_H

#include "net_sys.h"
#include "net_err.h"


/**
 * @brief 定义封装的锁类型
 */
typedef enum _nlocker_type_t {
    NLOCKER_NONE = 0,  //无锁
    NLOCKER_THREAD,  //线程互斥锁
} nlocker_type_t;


/**
 * @brief 封装协议栈使用的锁结构
 */
typedef struct _nlocker_t {

    nlocker_type_t type;  //锁类型

    union {
        sys_mutex_t mutex;  //互斥锁
    };  //封装的锁,根据类型不同，使用不同的锁

} nlocker_t;


net_err_t nlocker_init(nlocker_t *locker, nlocker_type_t type);

void nlocker_destroy(nlocker_t *locker);
void nlocker_lock(nlocker_t *locker);
void nlocker_unlock(nlocker_t *locker);


#endif  // NLOCKER_H