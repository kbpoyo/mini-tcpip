/**
 * @file exmsg.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 消息队列工作模块
 * @version 0.1
 * @date 2024-05-12
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "exmsg.h"

#include "dbg.h"
#include "fixq.h"
#include "mblock.h"
#include "net_sys.h"

static void *msg_tbl[EXMSG_MSG_CNT];  // 消息队列缓冲区，存放消息指针
static fixq_t msg_queue;              // 消息队列

static exmsg_t msg_buffer[EXMSG_MSG_CNT];  // 消息结构缓冲区,存放自定义消息结构
static mblock_t msg_mblock;  // 消息结构缓冲区内存块管理对象

/**
 * @brief 初始化消息队列工作模块
 *
 * @return net_err_t
 */
net_err_t exmsg_module_init(void) {
  dbg_info(DBG_EXMSG, "init exmsg module....");

  // 初始化使用的消息队列
  net_err_t err =
      fixq_init(&msg_queue, msg_tbl, EXMSG_MSG_CNT, EXMSG_LOCKER_TYPE);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_EXMSG, "fixq init failed.");
    return err;
  }

  // 初始化消息结构缓冲区内存块管理对象
  err = mblock_init(&msg_mblock, msg_buffer, sizeof(exmsg_t), EXMSG_MSG_CNT,
                    EXMSG_LOCKER_TYPE);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_EXMSG, "mblock init failed.");
    return err;
  }

  dbg_info(DBG_EXMSG, "init exmsg module ok.");
  return NET_ERR_OK;
}

static void work_thread(void *arg) {
  dbg_info(DBG_EXMSG, "exmsg work thread start....");

  while (1) {
    // 以阻塞方式从消息队列中接收消息
    exmsg_t *msg = (exmsg_t *)fixq_recv(&msg_queue, 0);

    // 根据消息类型进行相应的处理
    plat_printf("recv a msg: id = %d\n", msg->id);

    // 释放消息结构的内存块
    mblock_free(&msg_mblock, msg);
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
  msg->id = id++;

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
