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
#include "ipv4.h"
#include "mblock.h"
#include "net_plat.h"
#include "net_sys.h"
#include "netif.h"
#include "timer.h"

static void *msg_tbl[EXMSG_MSG_CNT];  // 消息队列缓冲区，存放消息指针
static fixq_t msg_queue;              // 消息队列

static exmsg_t msg_buffer[EXMSG_MSG_CNT];  // 消息结构缓冲区,存放自定义消息结构
static mblock_t msg_mblock;  // 消息结构缓冲区内存块管理对象

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

net_err_t exmsg_func_exec(exmsg_func_t func, void *arg) {
  // 创建函数执行请求的内部消息对象
  // 该消息对象需要在工作线程处理完成后，获取执行结果
  // 所以不能由工作线程释放，直接由请求线程在当前栈空间中创建与释放
  msg_func_t msg;
  msg.func = func;
  msg.arg = arg;
  msg.error = NET_ERR_OK;
  msg.sem = sys_sem_create(0);
  if (msg.sem == SYS_SEM_INVALID) {
    dbg_error(DBG_EXMSG, "msg func sem create failed.");
    return NET_ERR_EXMSG;
  }

  // 将消息对象封装成消息结构
  exmsg_t *exmsg = exmsg_alloc();
  if (!exmsg) {
    dbg_error(DBG_EXMSG, "msg func alloc failed.");
    // 释放信号量
    sys_sem_free(msg.sem);
    return NET_ERR_EXMSG;
  }
  exmsg->type = EXMSG_FUNC_EXEC;
  exmsg->msg_func = &msg;

  // 有可能有多个线程同时向消息队列中放入消息，所以以阻塞的方式，将消息结构放入消息队列
  net_err_t err = fixq_put(&msg_queue, exmsg, 0);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_EXMSG, "msg func send failed.");
    // 释放信号量
    sys_sem_free(msg.sem);
    exmsg_free(exmsg);
    return err;
  }

  // 等待工作线程执行完当前函数
  sys_sem_wait(msg.sem, 0);

  // 工作线程执行完后，返回值记录在msg.error中
  return msg.error;
}

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

  net_err_t err = fixq_put(&msg_queue, msg, -1);
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

  // 以非阻塞的方式从网络接口接收数据包
  pktbuf_t *buf = (pktbuf_t *)0;
  while ((buf = netif_recvq_get(netif, -1))) {  //!!! 取数据包
    dbg_info(DBG_EXMSG, "%s: received packet.", netif->name);

    // 重置数据包的访问位置
    pktbuf_acc_reset(buf);
    // 处理数据包
    if (netif->link_layer) {
      err =
          netif->link_layer->recv(netif, buf);  // 交给链路层处理接收到的数据包
      if (err != NET_ERR_OK) {
        dbg_warning(DBG_EXMSG, "loss packet: link layer recv failed.");
        pktbuf_free(buf);  //!!! 释放数据包
      }
    } else {
      // 该接口无链路层处理函数(loop接口)，直接将数据包交给ipv4层处理
      err = ipv4_recv(netif, buf);
      if (err != NET_ERR_OK) {
        dbg_warning(DBG_EXMSG, "loss packet: ipv4 recv failed.");
        pktbuf_free(buf);  //!!! 释放数据包
      }
    }
  }
}

/**
 * @brief func_exec消息处理函数
 *
 * @param msg
 */
static void exmsg_handle_func_exec(exmsg_t *msg) {
  dbg_info(DBG_EXMSG, "begin call func: %p", msg->msg_func->func);
  // 获取函数执行请求的内部消息对象
  msg_func_t *msg_func = msg->msg_func;
  // 执行函数
  msg_func->error = msg_func->func(msg_func);

  // 函数执行完毕，通知请求线程
  sys_sem_notify(msg_func->sem);

  dbg_info(DBG_EXMSG, "end call func: %p", msg->msg_func->func);
}

/**
 * @brief 消息队列模块的工作线程，用于处理消息事件和定时器事件
 *
 * @param arg
 */
static void exmsg_work_thread(void *arg) {
  dbg_info(DBG_EXMSG, "exmsg work thread is running....");

  // 获取系统当前时间
  net_time_t sys_time;
  sys_time_curr(&sys_time);

  while (1) {
    // 以阻塞方式从消息队列中接收消息, 并设置超时时间为当前最先到期的定时器时间
    exmsg_t *msg = (exmsg_t *)fixq_get(&msg_queue, net_timer_first_tmo());
    if (msg) {                  // 有消息
      switch (msg->type) {      // 根据消息类型处理消息
        case EXMSG_NETIF_RECV:  // 网络接口接收到数据包
          exmsg_handle_netif_recv(msg);
          break;
        case EXMSG_FUNC_EXEC:  // 外部程序请求执行函数
          exmsg_handle_func_exec(msg);
          break;
        default:
          dbg_warning(DBG_EXMSG, "unknown msg type.");
          break;
      }

      // TODO：当前线程已处理完消息，需要释放消息结构
      exmsg_free(msg);
    } else {  // 没有消息
      dbg_warning(DBG_EXMSG, "no msg.");
    }

    // 获取本次消息处理耗时, 并扫描定时器
    int diff_ms = sys_time_goes(&sys_time);
    net_timer_check_tmo(diff_ms);
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
