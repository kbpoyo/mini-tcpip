/**
 * @file netif.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 网络接口模块(适配器)
 * @version 0.1
 * @date 2024-05-21
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "netif.h"

#include "dbg.h"
#include "fixq.h"
#include "mblock.h"
#include "nlist.h"
#include "nlocker.h"

static netif_t netif_buffer[NETIF_MAX_CNT];  // 网络接口缓冲区
static mblock_t netif_mblock;  // 空闲网络接口内存块管理对象
static nlist_t netif_list;     // 已使用的网络接口链表

/**
 * @brief 初始化网络接口模块
 *
 * @return net_err_t
 */
net_err_t netif_module_init(void) {
  dbg_info(DBG_NETIF, "init netif module....");

  // 初始化网络接口内存块管理对象
  mblock_init(&netif_mblock, netif_buffer, sizeof(netif_t), NETIF_MAX_CNT,
              NLOCKER_NONE);

  // 初始化网络接口链表
  nlist_init(&netif_list);

  dbg_info(DBG_NETIF, "init netif module ok.");

  return NET_ERR_OK;
}

/**
 * @brief 打开一个网络接口
 *
 * @param dev_name
 * @return netif_t*
 */
netif_t *netif_open(const char *dev_name, const netif_ops_t *ops,
                    void *ops_data) {
  // 从内存块中分配一个网络接口对象
  netif_t *netif = (netif_t *)mblock_alloc(&netif_mblock, -1);
  if (netif == (netif_t *)0) {
    dbg_error(DBG_NETIF, "no memory for netif.\n");
    return (netif_t *)0;
  }

  // 初始化网络接口对象
  ipaddr_set_any(&(netif->ipaddr));   // 设置ip地址为任意地址
  ipaddr_set_any(&(netif->netmask));  // 设置子网掩码为任意地址
  ipaddr_set_any(&(netif->gateway));  // 设置网关地址为任意地址

  plat_strncpy(netif->name, dev_name, NETIF_NAME_SIZE);  // 设置接口名称
  netif->name[NETIF_NAME_SIZE - 1] = '\0';  // 设置字符串结束符, 确保内存安全

  plat_memset(&(netif->hwaddr), 0, sizeof(netif_hwaddr_t));  // 清空硬件地址
  netif->type = NETIF_TYPE_NONE;  // 接口在打开时不知道类型, 先设置为无类型
  netif->mtu = 0;                 // 最大传输单元，暂时设置为0

  // 初始化节点, 用于挂载到已使用网络接口的链表(netif_list)中
  nlist_node_init(&(netif->node));

  net_err_t err =
      fixq_init(&(netif->recv_fixq), netif->recv_buf, NETIF_RECV_BUFSIZE,
                NLOCKER_THREAD);  // 初始化接收缓冲队列
  if (err != NET_ERR_OK) {
    dbg_error(DBG_NETIF, "recv_fixq failed.\n");
    goto init_failed;
  }

  err = fixq_init(&(netif->send_fixq), netif->send_buf, NETIF_SEND_BUFSIZE,
                  NLOCKER_THREAD);  // 初始化发送缓冲队列
  if (err != NET_ERR_OK) {
    dbg_error(DBG_NETIF, "send_fixq failed.\n");
    fixq_destroy(&(netif->recv_fixq));  // 销毁接收缓冲队列
    goto init_failed;
  }

  err = ops->open(netif, ops_data);  // 使用特定操作方法对接口进行进一步的初始化
  if (err != NET_ERR_OK) {
    dbg_error(DBG_NETIF, "netif ops open failed or netif is typeless.\n");
    fixq_destroy(&(netif->recv_fixq));  // 销毁接收缓冲队列
    fixq_destroy(&(netif->send_fixq));  // 销毁发送缓冲队列
    goto init_failed;
  }

  netif->state = NETIF_STATE_OPENED;  // 设置接口状态为打开
  netif->ops = ops;                   // 设置接口操作方法
  netif->ops_data = ops_data;         // 设置接口操作数据
  nlist_insert_last(&netif_list,
                    &(netif->node));  // 将接口挂载到已使用网络接口链表中

  return netif;

init_failed:
  mblock_free(&netif_mblock, netif);  // 释放网络接口对象
  return (netif_t *)0;
}