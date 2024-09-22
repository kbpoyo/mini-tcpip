/**
 * @file route.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 路由表模块
 * @version 0.1
 * @date 2024-09-22
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "netif.h"

// 定义路由表项结构
typedef struct _route_entry_t {
  nlist_node_t node;  // 用于挂载到全局路由表链表的节点

  ipaddr_t dest_net;  // 目的子网
  ipaddr_t mask;      // 子网掩码
  ipaddr_t
      next_hop;  // 下一跳地址
                 //(若在同一子网内，则当前子网为目的子网，目标地址在当前链路上)
                 //(若不在同一子网内，则下一跳地址为网关地址)
  netif_t *netif;  // 发送数据的网络接口
} route_entry_t;

net_err_t route_init(void);
net_err_t route_add(ipaddr_t *dest_net, ipaddr_t *mask, ipaddr_t *next_hop,
                    netif_t *netif);
void route_remove(ipaddr_t *dest_net, ipaddr_t *mask);