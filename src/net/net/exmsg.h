#ifndef EXMSG_H
#define EXMSG_H

#include "net_err.h"
#include "netif.h"

/**
 * @brief 网络接口的消息结构
 *
 */
typedef struct _msg_netif_t {
  netif_t *netif;
} msg_netif_t;

/**
 * @brief 消息结构
 *
 */
typedef struct _exmsg_t {
  nlist_node_t node;
  enum {    // 消息类型
    EXMSG_NETIF_RECV = 0,  // 接收到数据
    EXMSG_NETIF_SEND,      // 发送数据
    EXMSG_NETIF_FUNC,      // 工作函数调用
  } type;

  union {   // 消息数据
    msg_netif_t msg_netif;
  };
} exmsg_t;

net_err_t exmsg_module_init(void);
net_err_t exmsg_start(void);
net_err_t exmsg_netif_recv(netif_t *netif);

#endif  // EXMSG_H