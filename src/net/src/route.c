/**
 * @file route.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 路由表模块
 * @version 0.1
 * @date 2024-09-22
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "route.h"

#include "mblock.h"

static route_entry_t route_tbl[ROUTE_ENTRY_MAXCNT];  // 路由表
static nlist_t route_list;                           // 路由表链表
static mblock_t route_mblock;  // 路由表内存块管理对象

#if DBG_DISP_ENABLED(DBG_ROUTE)

void route_list_disp(void) {
  nlist_node_t *node = (nlist_node_t *)0;
  route_entry_t *rt_entry = (route_entry_t *)0;
  int index = 0;

  plat_printf("---------------route table:---------------\n");
  nlist_for_each(node, &route_list) {
    rt_entry = nlist_entry(node, route_entry_t, node);
    plat_printf("[%d]:", index++);
    netif_dum_ip(" dest net: ", &rt_entry->dest_net);
    netif_dum_ip(" mask: ", &rt_entry->mask);
    netif_dum_ip(" next hop: ", &rt_entry->next_hop);
    plat_printf(" netif: %s\n", rt_entry->netif->name);
  }
}

#else
#define route_list_disp()
#endif

/**
 * @brief 从内存块管理对象中分配一个路由表表项
 *
 * @return route_entry_t*
 */
static route_entry_t *route_alloc(void) {
  route_entry_t *rt_entry = (route_entry_t *)mblock_alloc(&route_mblock, -1);
  if (!rt_entry) {
    dbg_error(DBG_ROUTE, "no free route entry.");
  }

  return rt_entry;
}

/**
 * @brief 释放一个路由表表项
 *
 * @param rt_entry
 */
static void route_free(route_entry_t *rt_entry) {
  // 将路由表表项从全局路由表链表中移除
  nlist_remove(&route_list, &rt_entry->node);
  // 释放路由表表项内存
  mblock_free(&route_mblock, rt_entry);
}

/**
 * @brief 初始化全局路由表
 *
 * @return net_err_t
 */
net_err_t route_init(void) {
  // 初始化路由表链表
  nlist_init(&route_list);

  // 初始化路由表内存块管理对象
  return mblock_init(&route_mblock, route_tbl, sizeof(route_entry_t),
                     ROUTE_ENTRY_MAXCNT, NLOCKER_NONE);
}

/**
 * @brief 添加一个路由表表项
 *
 * @param dest_net 目的子网
 * @param mask 子网掩码
 * @param next_hop 下一跳地址
 * @param netif 发送数据的网络接口
 * @return net_err_t
 */
net_err_t route_add(ipaddr_t *dest_net, ipaddr_t *mask, ipaddr_t *next_hop,
                    netif_t *netif) {
  // 分配一个路由表表项
  route_entry_t *rt_entry = route_alloc();
  if (!rt_entry) {
    dbg_error(DBG_ROUTE, "route entry alloc failed.");
    return NET_ERR_ROUTE;
  }

  // 初始化路由表表项
  ipaddr_copy(&rt_entry->dest_net, dest_net);
  ipaddr_copy(&rt_entry->mask, mask);
  ipaddr_copy(&rt_entry->next_hop, next_hop);
  rt_entry->netif = netif;

  // 将路由表表项添加到全局路由表链表中
  nlist_insert_last(&route_list, &rt_entry->node);

  route_list_disp();

  return NET_ERR_OK;
}

/**
 * @brief 删除一个路由表表项
 *
 * @param dest_net
 * @param mask
 */
void route_remove(ipaddr_t *dest_net, ipaddr_t *mask) {
  nlist_node_t *node = (nlist_node_t *)0;

  // 遍历全局路由表链表
  nlist_for_each(node, &route_list) {
    route_entry_t *rt_entry = nlist_entry(node, route_entry_t, node);
    if (ipaddr_is_equal(&rt_entry->dest_net, dest_net) &&
        ipaddr_is_equal(&rt_entry->mask, mask)) {  // 找到对应的路由表表项
      route_free(rt_entry);
      route_list_disp();
      return;
    }
  }
}