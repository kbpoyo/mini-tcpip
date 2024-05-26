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
#include "netif.h"

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

/**
 * @brief 分配一个消息结构
 *
 * @return exmsg_t*
 */
static exmsg_t *exmsg_alloc(void) {
  exmsg_t *msg = (exmsg_t *)mblock_alloc(&msg_mblock, -1);
  if (!msg) {
    dbg_warning(DBG_EXMSG, "no free msg buffer.");
    return (void *)0;
  }

  return msg;
}

/**
 * @brief 释放一个消息结构
 *
 * @param msg
 */
static void exmsg_free(exmsg_t *msg) {
  switch (msg->type) {
    case EXMSG_NETIF_RECV:
      break;
    default:
      dbg_warning(DBG_EXMSG, "unknown msg type.");
      break;
  }

  mblock_free(&msg_mblock, msg);
}

/**
 * @brief 从网卡接收数据后，发送到消息队列
 *
 * @return net_err_t
 */
net_err_t exmsg_netif_recv(netif_t *netif) {
  exmsg_t *msg = exmsg_alloc();
  if (!msg) {
    dbg_warning(DBG_EXMSG, "msg alloc failed.");
    return NET_ERR_MEM;
  }

  msg->type = EXMSG_NETIF_RECV;  // 设置消息类型为接收到数据包
  msg->msg_netif.netif = netif;  // 设置要传递的消息数据(接收到数据包的网络接口)

  net_err_t err = fixq_send(&msg_queue, msg, -1);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_EXMSG, "msg queue send failed.");
    exmsg_free(msg);
    return err;
  }

  // TODO: 消息已发送到消息队列，msg不由当前线程管理，不需要释放msg

  return NET_ERR_OK;
}

/**
 * @brief 处理消息: 网络接口接收到数据包
 * 该函数对数据包的生命周期进行部分管理
 * 当数据包传给外部协议层处理时，数据包的生命周期由外部协议层管理
 * 当外部协议层处理成功时，数据包由外部协议层释放
 * 当外部协议层处理失败时，数据包由该函数释放
 * @param msg
 */
static void exmsg_handle_netif_recv(exmsg_t *msg) {
  net_err_t err = NET_ERR_OK;

  // 获取网络接口
  netif_t *netif = msg->msg_netif.netif;

  // 从网络接口接收数据包
  pktbuf_t *buf = (pktbuf_t *)0;
  while ((buf = netif_recvq_get(netif, -1))) {  // 以非阻塞方式获取数据包

    dbg_info(DBG_EXMSG, "%s: received packet.", netif->name);

    // 处理数据包
    if (netif->link_layer) {
      err =
          netif->link_layer->recv(netif, buf);  // 交给链路层处理接收到的数据包
      if (err != NET_ERR_OK) {
        dbg_warning(DBG_EXMSG, "loss packet: link layer recv failed.");
        pktbuf_free(buf);  // 释放数据包
      }
    } else {  // TODO: 暂时没有其它协议层的处理
      pktbuf_free(buf);  // 释放数据包
    }
  }
}

/**
 * @brief 消息队列模块的工作线程，用于处理消息
 *
 * @param arg
 */
static void exmsg_work_thread(void *arg) {
  dbg_info(DBG_EXMSG, "exmsg work thread is running....");

  while (1) {
    // 以阻塞方式从消息队列中接收消息
    exmsg_t *msg = (exmsg_t *)fixq_recv(&msg_queue, 0);
    if (!msg) {
      dbg_warning(DBG_EXMSG, "no msg.");
      continue;
    }

    switch (msg->type) {
      case EXMSG_NETIF_RECV:
        exmsg_handle_netif_recv(msg);
        break;
      default:
        dbg_warning(DBG_EXMSG, "unknown msg type.");
        break;
    }

    // TODO：当前线程已处理完消息，需要释放消息结构
    exmsg_free(msg);
  }
}

/**
 * @brief 启动消息队列模块
 *
 * @return net_err_t
 */
net_err_t exmsg_start(void) {
  sys_thread_t thread = sys_thread_create(exmsg_work_thread, (void *)0);

  if (thread == SYS_THREAD_INVALID) {
    return NET_ERR_SYS;
  }
}
