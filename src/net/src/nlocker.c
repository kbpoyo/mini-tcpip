/**
 * @file nlocker.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-05-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "nlocker.h"

/**
 * @brief 初始化锁
 * 
 * @param locker 
 * @param type 
 * @return net_err_t 
 */
net_err_t nlocker_init(nlocker_t *locker, nlocker_type_t type) {
    if (type == NLOCKER_THREAD) {
        sys_mutex_t mutex = sys_mutex_create();
        if (mutex == SYS_MUTEX_INVALID) {
            return NET_ERR_SYS;
        }

        locker->mutex = mutex;
    }

    locker->type = type;

    return NET_ERR_OK;
}

/**
 * @brief 销毁锁
 * 
 * @param locker 
 */
void nlocker_destroy(nlocker_t *locker) {
    if (locker->type == NLOCKER_THREAD) {
        sys_mutex_free(locker->mutex);
    }
}

/**
 * @brief 加锁
 * 
 * @param locker 
 */
void nlocker_lock(nlocker_t *locker) {
    if (locker->type == NLOCKER_THREAD) {
        sys_mutex_lock(locker->mutex);
    }

}

/**
 * @brief 解锁
 * 
 * @param locker 
 */
void nlocker_unlock(nlocker_t *locker) {
    if (locker->type == NLOCKER_THREAD) {
        sys_mutex_unlock(locker->mutex);
    }
}