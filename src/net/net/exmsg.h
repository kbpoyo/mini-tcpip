#ifndef EXMSG_H
#define EXMSG_H

#include "net_err.h"
#include "netif.h"

/**
 * @brief 网络接口收到数据包的内部消息对象
 *
 */
typedef struct _msg_netif_t {
  netif_t *netif;
} msg_netif_t;


struct _msg_func_t;
// 定义函数执行请求的函数指针类型
typedef net_err_t (*exmsg_func_t)(struct _msg_func_t *msg);

/**
 * @brief 函数执行请求的内部消息对象
 * 
 */
typedef struct _msg_func_t {
  exmsg_func_t func; // 请求执行的函数对象
  void *arg; // 函数参数
  net_err_t error; // 函数执行结果
  sys_sem_t sem; // 用于同步请求的信号量：等待工作线程执行完当前函数
} msg_func_t;

/**
 * @brief 消息结构
 *
 */
typedef struct _exmsg_t {
  nlist_node_t node;
  enum {    // 消息类型
    EXMSG_NETIF_RECV = 0,  // 网络接口接收到数据
    EXMSG_FUNC_EXEC,      // 外部程序请求执行函数
  } type;

  union {   // 消息数据
    msg_netif_t msg_netif;
    msg_func_t *msg_func;
  };
} exmsg_t;

net_err_t exmsg_module_init(void);
net_err_t exmsg_start(void);
net_err_t exmsg_netif_recv(netif_t *netif);
net_err_t exmsg_func_exec(exmsg_func_t func, void *arg);

#endif  // EXMSG_H