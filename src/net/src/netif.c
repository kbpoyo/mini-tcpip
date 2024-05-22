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
#include "exmsg.h"
#include "fixq.h"
#include "mblock.h"
#include "nlist.h"
#include "nlocker.h"
#include "pktbuf.h"

// 网络接口状态字符串
static const char *netif_state_str[] = {[NETIF_STATE_OPENED] = "opened",
                                        [NETIF_STATE_ACVTIVE] = "active",
                                        [NETIF_STATE_CLOSED] = "closed"};
// 网络接口类型字符串
static const char *netif_type_str[] = {[NETIF_TYPE_NONE] = "none",
                                       [NETIF_TYPE_LOOP] = "loop",
                                       [NETIF_TYPE_ETHER] = "ether"};

static netif_t netif_buffer[NETIF_MAX_CNT];  // 网络接口缓冲区
static mblock_t netif_mblock;  // 空闲网络接口内存块管理对象
static nlist_t netif_list;     // 已使用的网络接口链表

static netif_t *netif_default =
    (netif_t *)0;  // 默认网络接口(当前操作的网络接口)

#if DBG_DISP_ENABLED(DBG_NETIF)

void display_netif_list(void) {  // 显示网络接口列表
  plat_printf("netif list:\n");

  nlist_node_t *node;
  nlist_for_each(node, &netif_list) {
    netif_t *netif = nlist_entry(node, netif_t, node);
    plat_printf("%s, %s, %s, mtu=%d\n", netif->name,
                netif_type_str[netif->type], netif_state_str[netif->state],
                netif->mtu);

    netif_dum_hwaddr("hwaddr: ", &(netif->hwaddr));  // 显示硬件地址
    netif_dum_ip(" ip: ", &(netif->ipaddr));         // 显示ip地址
    netif_dum_ip(" mask: ", &(netif->netmask));      // 显示子网掩码
    netif_dum_ip(" gateway: ", &(netif->gateway));   // 显示网关地址

    plat_printf("\n");
  }
}

#else
#define display_netif_list()
#endif

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

  // 设置默认网络接口为空
  netif_default = (netif_t *)0;

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

/**
 * @brief 关闭网络接口
 *
 * @param netif
 * @return net_err_t
 */
net_err_t netif_close(netif_t *netif) {
  if (netif == (netif_t *)0) {
    return NET_ERR_PARAM;
  }

  // 若网络接口为激活状态，则不能关闭
  if (netif->state == NETIF_STATE_ACVTIVE) {
    dbg_error(DBG_NETIF, "netif is active.\n");
    return NET_ERR_STATE;
  }

  // 调用特定操作方法关闭网络接口
  netif->ops->close(netif);

  // 设置网络接口状态为关闭
  netif->state = NETIF_STATE_CLOSED;

  // 从已使用网络接口链表中移除
  nlist_remove(&netif_list, &(netif->node));

  // 释放网络接口内存块
  mblock_free(&netif_mblock, netif);

  display_netif_list();
}

/**
 * @brief 设置网络接口的ip地址，子网掩码和网关地址
 *
 * @param netif
 * @param ip
 * @param mask
 * @param gateway
 * @return net_err_t
 */
net_err_t netif_set_addr(netif_t *netif, const ipaddr_t *ip,
                         const ipaddr_t *mask, const ipaddr_t *gateway) {
  ipaddr_copy(&(netif->ipaddr), ip ? ip : ipaddr_get_any());
  ipaddr_copy(&(netif->netmask), mask ? mask : ipaddr_get_any());
  ipaddr_copy(&(netif->gateway), gateway ? gateway : ipaddr_get_any());

  return NET_ERR_OK;
}

/**
 * @brief 设置网络接口的硬件MAC地址
 *
 * @param netif
 * @param hwaddr
 * @param len
 * @return net_err_t
 */
net_err_t netif_set_hwaddr(netif_t *netif, const uint8_t *hwaddr,
                           uint32_t len) {
  if (hwaddr == (uint8_t *)0 || len > NETIF_HWADDR_SIZE) {
    return NET_ERR_PARAM;
  }

  plat_memcpy(&(netif->hwaddr), hwaddr, len);
  netif->hwaddr.valid_len = len;

  return NET_ERR_OK;
}

/**
 * @brief 设置网络接口为激活状态
 *
 * @param netif
 * @return net_err_t
 */
net_err_t netif_set_acticve(netif_t *netif) {
  if (netif->state != NETIF_STATE_OPENED) {
    dbg_error(DBG_NETIF, "netif is not opened.\n");
    return NET_ERR_STATE;
  }

  netif->state = NETIF_STATE_ACVTIVE;

  // 若默认网络接口为空，则将当前激活的网络接口设置为默认网络接口(不能将环回接口设置为默认接口)
  if (netif_default == (netif_t *)0 && netif->type != NETIF_TYPE_LOOP) {
    netif_set_default(netif);
  }

  display_netif_list();
  return NET_ERR_OK;
}

/**
 * @brief 设置网络接口为非激活状态
 *
 * @param netif
 * @return net_err_t
 */
net_err_t netif_set_inactive(netif_t *netif) {
  if (netif->state != NETIF_STATE_ACVTIVE) {
    dbg_error(DBG_NETIF, "netif is not active.\n");
    return NET_ERR_STATE;
  }

  void *pkt = 0;
  // 释放接收队列中的数据
  while ((pkt = fixq_recv(&(netif->recv_fixq), -1)) != (void *)0) {
    pktbuf_free(pkt);
  }

  // 释放发送队列中的数据
  while ((pkt = fixq_recv(&(netif->send_fixq), -1)) != (void *)0) {
    pktbuf_free(pkt);
  }

  // 若该网络接口为默认网络接口，则将默认网络接口置为空
  if (netif == netif_default) {
    netif_default = (netif_t *)0;
  }

  // 设置网络接口状态为打开
  netif->state = NETIF_STATE_OPENED;

  display_netif_list();
  return NET_ERR_OK;
}

/**
 * @brief 设置默认网络接口
 *
 * @param netif
 */
void netif_set_default(netif_t *netif) { netif_default = netif; }

/**
 * @brief 向接收队列中放入数据包
 *
 * @param netif
 * @param buf
 * @param tmo
 * @return net_err_t
 */
net_err_t netif_recvq_put(netif_t *netif, pktbuf_t *buf, int tmo) {
  net_err_t err = fixq_send(&(netif->recv_fixq), (void *)buf, tmo);
  if (err < 0) {
    dbg_warning(DBG_NETIF, "netif recv queue full.\n");
    return NET_ERR_FULL;
  }

  // 测试用, 从网卡接收队列中取出数据包,
  // 发送到消息队列(网卡管理线程与数据包处理线程)中
  exmsg_netif_recv(netif);

  return NET_ERR_OK;
}
/**
 * @brief 从接收队列中获取数据包
 *
 * @param netif
 * @param tmo
 * @return pktbuf_t*
 */
pktbuf_t *netif_recvq_get(netif_t *netif, int tmo) {
  pktbuf_t *buf = (pktbuf_t *)fixq_recv(&(netif->recv_fixq), tmo);

  if (buf == (pktbuf_t *)0) {  // 接收队列为空
    dbg_info(DBG_NETIF, "netif recv queue empty.\n");
    return (pktbuf_t *)0;
  }

  pktbuf_acc_reset(buf);  // 重置数据包的访问位置
  return buf;
}

/**
 * @brief 向发送队列中放入数据包
 *
 * @param netif
 * @param buf
 * @param tmo
 * @return net_err_t
 */
net_err_t netif_sendq_put(netif_t *netif, pktbuf_t *buf, int tmo) {
  net_err_t err = fixq_send(&(netif->send_fixq), (void *)buf, tmo);
  if (err < 0) {
    dbg_warning(DBG_NETIF, "netif send queue full.\n");
    return NET_ERR_FULL;
  }

  return NET_ERR_OK;
}

/**
 * @brief 从发送队列中获取数据包
 *
 * @param netif
 * @param tmo
 * @return pktbuf_t*
 */
pktbuf_t *netif_sendq_get(netif_t *netif, int tmo) {
  pktbuf_t *buf = (pktbuf_t *)fixq_recv(&(netif->send_fixq), tmo);

  if (buf == (pktbuf_t *)0) {  // 接收队列为空
    dbg_info(DBG_NETIF, "netif send queue empty.\n");
    return (pktbuf_t *)0;
  }

  pktbuf_acc_reset(buf);  // 重置数据包的访问位置
  return buf;
}

/**
 * @brief 向网络接口发送数据包
 * 
 * @param netif 
 * @param ipaddr 
 * @param buf 
 * @return net_err_t 
 */
net_err_t netif_send(netif_t *netif, ipaddr_t *ipaddr, pktbuf_t *buf) {
    if (netif == (netif_t *)0 || buf == (pktbuf_t *)0) {
        return NET_ERR_PARAM;
    }

    // 将数据包放到发送队列中
    net_err_t err = netif_sendq_put(netif, buf, -1);
    if (err != NET_ERR_OK) {
        dbg_info(DBG_NETIF, "pktbuf send failed.\n");
        return err;
    }

    // 调用网络接口特定的操作方法发送数据包
    return netif->ops->send(netif);
 
}