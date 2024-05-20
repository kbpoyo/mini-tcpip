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
} netif_type_t;

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

  netif_type_t type;  // 接口类型
  int mtu;            // 最大传输单元

  enum {
    NETIF_STATE_CLOSED = 0,  // 接口关闭
    NETIF_STATE_OPENED,      // 接口打开
    NETIF_STATE_ACVTIVE,     // 接口激活
  } state;                   // 接口状态

  fixq_t recv_fixq;                  // 接收缓冲队列
  void *recv_buf[NETIF_RECV_BUFSIZE];  // 接收缓冲区
  fixq_t send_fixq;                  // 发送缓冲队列
  void *send_buf[NETIF_SEND_BUFSIZE];  // 发送缓冲区
} netif_t;

#endif