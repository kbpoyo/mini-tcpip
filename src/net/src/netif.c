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
#include "ether.h"
#include "exmsg.h"
#include "fixq.h"
#include "ipaddr.h"
#include "mblock.h"
#include "nlist.h"
#include "nlocker.h"
#include "pktbuf.h"
#include "protocol.h"
#include "route.h"

static const link_layer_t *link_layers[NETIF_TYPE_CNT];

// 网络接口状态字符串
static const char *netif_state_str[] = {
    [NETIF_STATE_OPENED] = "opened",
    [NETIF_STATE_ACVTIVE] = "active",
    [NETIF_STATE_CLOSED] = "closed",
    [NETIF_STATE_IPCONFLICT] = "ipconflict"};
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

    netif_dum_hwaddr("hwaddr: ", netif->hwaddr.addr,
                     netif->hwaddr.valid_len);      // 显示硬件地址
    netif_dum_ip(" ip: ", &(netif->ipaddr));        // 显示ip地址
    netif_dum_ip(" mask: ", &(netif->netmask));     // 显示子网掩码
    netif_dum_ip(" gateway: ", &(netif->gateway));  // 显示网关地址

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

  // 清理协议栈支持的链路层的回调接口指针数组
  plat_memset(link_layers, 0, sizeof(link_layers));

  dbg_info(DBG_NETIF, "init netif module ok.");

  return NET_ERR_OK;
}

/**
 * @brief 注册链路层的回调接口，
 *        设置link_layers[layer->type] = layer
 * @param layer
 * @return net_err_t
 */
net_err_t netif_layer_register(const link_layer_t *layer) {
  if (layer == (link_layer_t *)0) {
    dbg_error(DBG_NETIF, "layer is null.");
    return NET_ERR_PARAM;
  }

  netif_type_t type = layer->type;

  if (type < 0 || type >= NETIF_TYPE_CNT) {
    dbg_error(DBG_NETIF, "layer type error.");
    return NET_ERR_PARAM;
  }

  if (link_layers[type] !=
      (link_layer_t *)0) {  // 若已经注册过该类型的链路层回调接口, 则返回错误
    dbg_error(DBG_NETIF, "layer type %s has been registered.",
              netif_type_str[type]);
    return NET_ERR_EXIST;
  }

  // 记录链路层的回调接口的指针，以便后续用链路层类型获取对应的回调接口
  link_layers[type] = layer;
  return NET_ERR_OK;
}

/**
 * @brief 打开一个网络接口
 *
 * @param dev_name 设备名称
 * @param ops 该网络接口的特化操作方法集合的指针
 * @param ops_data  该网络接口的特化操作方法需要的数据
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
  plat_strncpy(netif->name, dev_name, NETIF_NAME_SIZE);  // 设置接口名称
  netif->name[NETIF_NAME_SIZE - 1] = '\0';  // 设置字符串结束符, 确保内存安全
  ipaddr_set_any(&(netif->ipaddr));   // 初始化ip
  ipaddr_set_any(&(netif->netmask));  // 初始化子网掩码
  ipaddr_set_any(&(netif->gateway));  // 初始化网关地址
  plat_memset(&(netif->hwaddr), 0, sizeof(netif_hwaddr_t));  // 清空硬件地址
  netif->type = NETIF_TYPE_NONE;  // 接口在打开时不知道类型, 先设置为无类型
  netif->mtu = 0;                 // 最大传输单元，暂时设置为0

  // 初始化节点, 用于挂载到已使用网络接口的链表(netif_list)中
  nlist_node_init(&(netif->node));

  // 初始化接收和发送缓冲队列
  net_err_t err =
      fixq_init(&(netif->recv_fixq), netif->recv_buf, NETIF_RECV_BUFSIZE,
                NLOCKER_THREAD);  // 初始化接收缓冲队列
  if (err != NET_ERR_OK) {
    dbg_error(DBG_NETIF, "init recv_fixq failed.\n");
    goto init_failed;
  }

  err = fixq_init(&(netif->send_fixq), netif->send_buf, NETIF_SEND_BUFSIZE,
                  NLOCKER_THREAD);  // 初始化发送缓冲队列
  if (err != NET_ERR_OK) {
    dbg_error(DBG_NETIF, "init send_fixq failed.");
    fixq_destroy(&(netif->recv_fixq));  // 销毁接收缓冲队列
    goto init_failed;
  }

  // 使用特定操作方法对接口进行进一步的初始化
  netif->ops = ops;                  // 设置接口操作方法
  netif->ops_data = ops_data;        // 设置接口操作数据
  err = ops->open(netif, ops_data);  // 打开之后可能会开启收发线程工作
                                     // 所以需要确保接收和发送队列已经初始化
  if (err != NET_ERR_OK) {
    dbg_error(DBG_NETIF, "netif %s open failed.", dev_name);
    fixq_destroy(&(netif->recv_fixq));  // 销毁接收缓冲队列
    fixq_destroy(&(netif->send_fixq));  // 销毁发送缓冲队列
    goto init_failed;
  }

  // 设置链路层回调接口
  netif->link_layer =
      link_layers[netif->type];  // 在调用特定操作方法之后网络接口的类型已经确定
  if (!(netif->link_layer) && netif->type != NETIF_TYPE_LOOP) {
    // 若未注册链路层回调接口(环回接口不需要进行链路层处理)
    dbg_error(DBG_NETIF, "no link layer for netif %s.", dev_name);
    goto init_failed;
  }

  // 将接口挂载到已使用网络接口链表中
  nlist_insert_last(&netif_list, &(netif->node));

  // 打开成功，设置接口状态为打开
  netif->state = NETIF_STATE_OPENED;

  return netif;

init_failed:  // 初始化失败，释放网络接口对象
  mblock_free(&netif_mblock, netif);
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
    dbg_error(DBG_NETIF, "netif close error, it is active.");
    return NET_ERR_STATE;
  }

  // 调用特定操作方法关闭网络接口
  netif->ops->close(netif);

  // 设置网络接口状态为关闭
  netif->state = NETIF_STATE_CLOSED;

  // 从已使用网络接口链表中移除
  if (nlist_is_mount(&netif->node)) {
    nlist_remove(&netif_list, &netif->node);
  }

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

  plat_memcpy(&(netif->hwaddr.addr), hwaddr, len);
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
    dbg_error(DBG_NETIF, "netif %s set active error: it not opened.\n",
              netif->name);
    return NET_ERR_NETIF;
  }

  // 若当前网络接口拥有链路层回调接口，则调用链路层回调接口的open方法以便进行链路层相关的初始化
  if (netif->link_layer) {
    net_err_t err = netif->link_layer->open(netif);
    if (err != NET_ERR_OK) {
      dbg_error(DBG_NETIF,
                "netif %s set active error: link layer open failed.\n",
                netif->name);
      return err;
    }
  }

  // 为该接口添加路由表项
  ipaddr_t ip =
      ipaddr_get_netnum(&netif->ipaddr, &netif->netmask);  // 获取接口所在子网号
  route_add(&ip, &(netif->netmask), ipaddr_get_any(),
            netif);  // 添加路由表项, 由该接口将发送到该子网的数据包转发到链路上
  ipaddr_from_str(&ip, "255.255.255.255");
  route_add(&netif->ipaddr, &ip, ipaddr_get_any(),
            netif);  // 添加路由表项, 由该接口处理送到自己的数据包

  // 激活成功，设置网络接口状态为激活
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
    dbg_error(DBG_NETIF, "netif %s set inactive error: it not active.\n",
              netif->name);
    return NET_ERR_NETIF;
  }

  void *pkt = 0;
  // 释放接收队列中的数据包
  while ((pkt = fixq_get(&(netif->recv_fixq), -1)) !=
         (void *)0) {              //!!! 获取数据包
    pktbuf_free((pktbuf_t *)pkt);  //!!! 释放数据包
  }

  // 释放发送队列中的数据包
  while ((pkt = fixq_get(&(netif->send_fixq), -1)) !=
         (void *)0) {              //!!! 获取数据包
    pktbuf_free((pktbuf_t *)pkt);  //!!! 释放数据包
  }

  // 若该网络接口为默认网络接口，则将默认网络接口置为空, 并删除默认路由表项
  if (netif == netif_default) {
    netif_default = (netif_t *)0;
    route_remove(ipaddr_get_any(), ipaddr_get_any());
  }

  // 若当前网络接口拥有链路层回调接口，则调用链路层回调接口的close方法以便进行链路层相关的清理
  if (netif->link_layer) {
    netif->link_layer->close(netif);
  }

  // 从路由表中删除该接口的路由表项
  ipaddr_t ip = ipaddr_get_netnum(&netif->ipaddr, &netif->netmask);
  route_remove(&ip, &(netif->netmask));
  ipaddr_from_str(&ip, "255.255.255.255");
  route_remove(&netif->ipaddr, &ip);

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
void netif_set_default(netif_t *netif) {
  if (!ipaddr_is_any(&netif->gateway)) {
    if (netif_default) {  // 若已经存在默认网络接口, 删除当前默认路由表项
      route_remove(ipaddr_get_any(), ipaddr_get_any());
    }

    // 设置默认网络接口，并将其当作默认路由表项添加到路由表中
    netif_default = netif;
    route_add(ipaddr_get_any(), ipaddr_get_any(), &netif->gateway,
              netif);  // 下一跳地址为该接口所在子网的网关地址
  }
}
netif_t *netif_get_default(void) { return netif_default; }

/**
 * @brief 向接网络接口的接收队列中放入数据包
 *
 * @param netif
 * @param buf 数据包
 * @param tmo 超时时间
 * @return net_err_t
 */
net_err_t netif_recvq_put(netif_t *netif, pktbuf_t *buf, int tmo) {
  net_err_t err =
      fixq_put(&(netif->recv_fixq), (void *)buf, tmo);  //!!! 数据包转交
  if (err < 0) {
    dbg_warning(DBG_NETIF, "netif %s pktbuf put error: recv queue is full.",
                netif->name);
    return NET_ERR_FULL;
  }

  // 测试用, 通知工作线程有数据到达
  // 发送到消息队列(网卡管理线程与数据包处理线程)中
  err = exmsg_netif_recv(netif);
  if (err != NET_ERR_OK) {
    // TODO:
    // 数据包已转交，但消息通知失败，该数据包可能一直在接收队列中，导致内存泄漏，需要进一步处理
    //  且不能直接返回错误，否则会导致上层调用者释放一个在接收队列中的数据包
    dbg_warning(DBG_NETIF, "exmsg netif recv failed.");
  }

  return NET_ERR_OK;
}
/**
 * @brief 从网络接口的接收队列中获取数据包
 *
 * @param netif
 * @param tmo 超时时间
 * @return pktbuf_t*
 */
pktbuf_t *netif_recvq_get(netif_t *netif, int tmo) {
  pktbuf_t *buf = (pktbuf_t *)fixq_get(&(netif->recv_fixq), tmo);

  if (buf == (pktbuf_t *)0) {  // 接收队列为空
    return (pktbuf_t *)0;
  }

  return buf;
}

/**
 * @brief 向发网络接口的发送队列中放入数据包
 *
 * @param netif
 * @param buf 数据包
 * @param tmo
 * @return net_err_t
 */
net_err_t netif_sendq_put(netif_t *netif, pktbuf_t *buf, int tmo) {
  net_err_t err =
      fixq_put(&(netif->send_fixq), (void *)buf, tmo);  //!!! 数据包转交
  if (err < 0) {
    dbg_warning(DBG_NETIF,
                "netif %s put buf into send_queue error: send queue is full.",
                netif->name);
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
  pktbuf_t *buf = (pktbuf_t *)fixq_get(&(netif->send_fixq), tmo);

  if (buf == (pktbuf_t *)0) {  // 接收队列为空
    dbg_warning(DBG_NETIF,
                "netif %s send_queue get buf error: send queue is empty.",
                netif->name);
  }

  return buf;
}

/**
 * @brief 通过网络接口发送数据包
 *
 * @param netif 网络接口
 * @param ipaddr 目的IP地址
 * @param buf 数据包
 * @return net_err_t
 */
net_err_t netif_send(netif_t *netif, const ipaddr_t *ipaddr, pktbuf_t *buf) {
  pktbuf_check_buf(buf);  // 检查数据包的有效性

  net_err_t err = NET_ERR_OK;

  if (netif->link_layer) {                              // 进行链路层处理
    err = netif->link_layer->send(netif, ipaddr, buf);  //!!! 数据包传递
    if (err != NET_ERR_OK) {
      dbg_warning(DBG_NETIF, "netif %s send buf error: link layer send failed.",
                  netif->name);
      return err;
    }

  } else {  // 接口没有链路层处理，直接发送数据包
    // 将数据包放到发送队列中
    err = netif_sendq_put(netif, buf, -1);  //!!! 数据包转交
    if (err != NET_ERR_OK) {
      dbg_warning(DBG_NETIF,
                  "netif %s send buf error: put buf into send_queue failed.",
                  netif->name);
      return err;
    }

    // 数据包处理完毕，调用网络接口特定的操作方法发送数据包
    netif->ops->send(netif);
    // 数据包已转交，该方法无论成功与否，都只需要返回NET_ERR_OK
  }
  return NET_ERR_OK;
}
