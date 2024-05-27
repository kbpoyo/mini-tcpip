/**
 * @file netif.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 网络接口模块(适配器)
 * @version 0.1
 * @date 2024-05-20
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef NETIF_H
#define NETIF_H

#include "fixq.h"
#include "ipaddr.h"
#include "nlist.h"
#include "pktbuf.h"

/**
 * @brief 网络接口硬件地址结构
 */
typedef struct _netif_hwaddr_t {
  uint8_t addr[NETIF_HWADDR_SIZE];  // 硬件地址
  uint8_t valid_len;                // 硬件地址的有效长度
} netif_hwaddr_t;

/**
 * @brief 网络接口类型
 *
 */
typedef enum _netif_type_t {
  NETIF_TYPE_NONE = 0,  // 无类型
  NETIF_TYPE_ETHER,     // 以太网
  NETIF_TYPE_LOOP,      // 环回网络
  NETIF_TYPE_WIFI,      // wifi
  NETIF_TYPE_PPP,       // ppp

  NETIF_TYPE_CNT,   // 特殊：记录网络接口类型数量
} netif_type_t;

struct _netif_t;

/**
 * @brief 对网络接口的操作方法进行抽象
 *
 */
typedef struct _netif_ops_t {
  net_err_t (*open)(struct _netif_t *netif, void *data);  // 初始化接口
  void (*close)(struct _netif_t *netif);                  // 关闭接口
  net_err_t (*send)(struct _netif_t *netif);              // 发送数据

} netif_ops_t;

struct _netif_t;

/**
 * @brief 定义链路层的统一抽象操作接口
 *
 */
typedef struct _link_layer_t {
  netif_type_t type;                          // 链路层类型
  net_err_t (*open)(struct _netif_t *netif);  // 完成链路层相关的初始化工作
  void (*close)(struct _netif_t *netif);                     // 关闭链路层
  net_err_t (*recv)(struct _netif_t *netif, pktbuf_t *buf);  // 从链路层接收数据
  net_err_t (*send)(struct _netif_t *netif, ipaddr_t *ipdest,
                    pktbuf_t *buf);  // 向链路层发送数据

} link_layer_t;

/**
 * @brief 网络接口结构
 */
typedef struct _netif_t {
  nlist_node_t node;           // 节点,用于挂载到链表中
  char name[NETIF_NAME_SIZE];  // 接口名称
  netif_hwaddr_t hwaddr;       // 硬件地址
  ipaddr_t ipaddr;             // ip地址
  ipaddr_t netmask;            // 子网掩码
  ipaddr_t gateway;            // 网关地址
  netif_type_t type;           // 接口类型
  int mtu;                     // 最大传输单元

  const link_layer_t *link_layer;  // 链路层回调接口

  enum {
    NETIF_STATE_CLOSED = 0,  // 接口关闭
    NETIF_STATE_OPENED,      // 接口打开
    NETIF_STATE_ACVTIVE,     // 接口激活
  } state;                   // 接口状态

  const netif_ops_t *ops;  // 接口操作方法
  void *ops_data;          // 接口操作数据

  fixq_t recv_fixq;                    // 接收缓冲队列
  void *recv_buf[NETIF_RECV_BUFSIZE];  // 接收缓冲区
  fixq_t send_fixq;                    // 发送缓冲队列
  void *send_buf[NETIF_SEND_BUFSIZE];  // 发送缓冲区
} netif_t;

net_err_t netif_module_init(void);

netif_t *netif_open(const char *dev_name, const netif_ops_t *ops,
                    void *ops_data);
net_err_t netif_close(netif_t *netif);

net_err_t netif_set_addr(netif_t *netif, const ipaddr_t *ip,
                         const ipaddr_t *mask, const ipaddr_t *gateway);
net_err_t netif_set_hwaddr(netif_t *netif, const uint8_t *hwaddr, uint32_t len);
net_err_t netif_set_acticve(netif_t *netif);
net_err_t netif_set_inactive(netif_t *netif);
void netif_set_default(netif_t *netif);

net_err_t netif_recvq_put(netif_t *netif, pktbuf_t *buf, int tmo);
pktbuf_t *netif_recvq_get(netif_t *netif, int tmo);
net_err_t netif_sendq_put(netif_t *netif, pktbuf_t *buf, int tmo);
pktbuf_t *netif_sendq_get(netif_t *netif, int tmo);

net_err_t netif_send(netif_t *netif, ipaddr_t *ipaddr, pktbuf_t *buf);

net_err_t netif_layer_register(const link_layer_t *layer);

/**
 * @brief 打印MAC地址信息
 *
 * @param msg
 * @param hwaddr
 * @param len
 */
static void netif_dum_hwaddr(const char *msg, const uint8_t* hwaddr, uint32_t len) {
  if (msg) {
    plat_printf("%s", msg);
  }

  if (hwaddr) {
    for (int i = 0; i < len; i++) {
      plat_printf("%02x", hwaddr[i]);
      if (i < len - 1) {
        plat_printf("-");
      }
    }
  }
}

/**
 * @brief 打印IP地址信息
 *
 * @param msg
 * @param ipadr
 */
static void netif_dum_ip(const char *msg, const ipaddr_t *ipaddr) {
  if (msg) {
    plat_printf("%s", msg);
  }

  if (ipaddr) {
    plat_printf("%d.%d.%d.%d", ipaddr->addr_bytes[0], ipaddr->addr_bytes[1],
                ipaddr->addr_bytes[2], ipaddr->addr_bytes[3]);
  }
}

#endif