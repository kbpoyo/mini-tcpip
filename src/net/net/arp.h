/**
 * @file arp.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief arp协议模块
 * @version 0.1
 * @date 2024-06-10
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef ARP_H
#define ARP_H

#include <stdint.h>

#include "ether.h"
#include "ipaddr.h"
#include "netif.h"
#include "nlist.h"

#define ARP_HW_ETHER 1    // 以太网硬件类型
#define ARP_OP_REQUEST 1  // arp请求
#define ARP_OP_REPLY 2    // arp响应

#pragma pack(1)
// arp协议包结构, arp设计目的可支持多种链路层和网络层,
// 但实际基本只用于以太网和ipv4
typedef struct _arp_pkt_t {
  uint16_t hw_type;                           // 硬件类型(链路层类型)
  uint16_t proto_type;                        // 协议类型(网络层)
  uint8_t hw_addr_size;                       // 硬件地址长度
  uint8_t proto_addr_size;                    // 协议地址长度
  uint16_t op_code;                           // 操作码
  uint8_t sender_hw_addr[ETHER_MAC_SIZE];     // 发送方硬件地址
  uint8_t sender_proto_addr[IPV4_ADDR_SIZE];  // 发送方协议地址
  uint8_t target_hw_addr[ETHER_MAC_SIZE];     // 目的硬件地址
  uint8_t target_proto_addr[IPV4_ADDR_SIZE];  // 目的协议地址
} arp_pkt_t;

#pragma pack()

// arp缓存表项结构
typedef struct _arp_entry_t {
  nlist_node_t node;  // 用于挂载的链表节点

  nlist_t buf_list;  // 数据包缓存链表
  netif_t *netif;    // 使用的网络接口

  int tmo;    // 超时时间
  int retry;  // 重试次数

  enum {
    NET_ARP_FREE,      // 空闲
    NET_ARP_WAITING,   // 等待响II
    NET_ARP_RESOLVED,  // 已获取响应
  } state;             // arp表项状态

  uint8_t ipaddr[IPV4_ADDR_SIZE];  // ip地址(ipv4)
  uint8_t hwaddr[ETHER_MAC_SIZE];  // 硬件地址(以太网mac地址)
} arp_entry_t;

net_err_t arp_module_init(void);

net_err_t arp_make_request(netif_t *netif, const uint8_t *dest_ipaddr_bytes);
net_err_t arp_make_gratuitous(netif_t *netif);
net_err_t arp_make_reply(netif_t *netif, pktbuf_t *buf);
net_err_t arp_make_probe(netif_t *netif);

net_err_t arp_recv(netif_t *netif, pktbuf_t *buf);
net_err_t arp_send(netif_t *netif, const uint8_t *dest_ipaddr_bytes, pktbuf_t *buf);


#endif  // ARP_H