/**
 * @file exmsg.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 启动消息队列模块
 * @version 0.1
 * @date 2024-05-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */


#include "exmsg.h"
#include "net_sys.h"
#include "fixq.h"
#include "mblock.h"
#include "dbg.h"

static void *msg_tbl[EXMSG_MSG_CNT];
static fixq_t msg_queue;

static exmsg_t msg_buffer[EXMSG_MSG_CNT];
static mblock_t msg_mblock;

/**
 * @brief 初始化消息队列模块 
 * 
 * @return net_err_t 
 */
net_err_t exmsg_init(void) {
    dbg_info(DBG_EXMSG, "exmsg init....");

    net_err_t err = fixq_init(&msg_queue, msg_tbl, EXMSG_MSG_CNT, EXMSG_LOCKER_TYPE);
    if (err != NET_ERR_OK) {
        dbg_error(DBG_EXMSG, "fixq init failed.");
        return err;
    }

    err = mblock_init(&msg_mblock, msg_buffer, sizeof(exmsg_t), EXMSG_MSG_CNT, EXMSG_LOCKER_TYPE);
    if (err != NET_ERR_OK) {
        dbg_error(DBG_EXMSG, "mblock init failed.");
        return err;
    }


    dbg_info(DBG_EXMSG, "exmsg init success....");
    return NET_ERR_OK;
}


static void work_thread(void *arg) {
    dbg_info(DBG_EXMSG, "exmsg work thread start....");

    while (1) {
        sys_sleep(10);
    }
}


/**
 * @brief 从网卡接收数据后，发送到消息队列
 * 
 * @return net_err_t 
 */
net_err_t exmsg_netif_in(void) {
    exmsg_t *msg = (exmsg_t *)mblock_alloc(&msg_mblock, -1);
    if (!msg) {
        dbg_warning(DBG_EXMSG, "no free msg.");
        return NET_ERR_MEM;
    }

    static int id = 0;
    msg->type = EXMSG_NETIF_IN;
    msg->id = id;

    net_err_t err = fixq_send(&msg_queue, msg, -1);
    if (err != NET_ERR_OK) {
        dbg_error(DBG_EXMSG, "fixq full.");
        mblock_free(&msg_mblock, msg);
        return err;
    }

    return NET_ERR_OK;
}


/**
 * @brief 启动消息队列模块
 * 
 * @return net_err_t 
 */
net_err_t exmsg_start(void) {
    sys_thread_t thread = sys_thread_create(work_thread, (void *)0);

    if (thread == SYS_THREAD_INVALID) {
        return NET_ERR_SYS;
    }
}

