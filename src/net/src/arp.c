/**
 * @file arp.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief arp协议模块
 * @version 0.1
 * @date 2024-06-10
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "arp.h"

#include "dbg.h"
#include "mblock.h"
#include "net_cfg.h"
#include "net_err.h"
#include "nlocker.h"
#include "tools.h"

static arp_entry_t cache_tbl[ARP_CACHE_SIZE];  // arp缓存表
static mblock_t cache_tbl_mblock;  // arp缓存表内存块管理对象
static nlist_t cache_tbl_list;     // 管理已分配的arp缓存表

#if DBG_DISP_ENABLED(DBG_ARP)

static void arp_cache_entry_display(arp_entry_t *entry) {
  ipaddr_t ipaddr;
  ipaddr.type = IPADDR_V4;
  plat_memcpy(ipaddr.addr_bytes, entry->ipaddr, IPV4_ADDR_SIZE);
  netif_dum_ip("\tip: ", &ipaddr);
  netif_dum_hwaddr("\tmac: ", entry->hwaddr, ETHER_MAC_SIZE);

  plat_printf("\ttmo: %d, retry: %d, state: %s pktbuf_cnt: %d\n", entry->tmo,
              entry->retry,
              entry->state == NET_ARP_RESOLVED ? "resolved" : "waitting",
              nlist_count(entry->buf_list));
  plat_printf("\n");
}

static void arp_tbl_display(void) {
  nlist_node_t *node;
  arp_entry_t *entry;

  plat_printf("---------------- arp cache table ----------------\n");
  nlist_for_each(node, &cache_tbl_list) {
    entry = nlist_entry(node, arp_entry_t, node);

    arp_cache_entry_display(entry);
  }
  plat_printf("-------------------------------------------------\n");
}

/**
 * @brief 打印arp数据包
 *
 * @param arp_pkt
 */
static void arp_pkt_display(arp_pkt_t *arp_pkt) {
  uint16_t op_code = net_ntohs(arp_pkt->op_code);
  plat_printf("---------------- arp packet ----------------\n");
  plat_printf(
      "\thw_type: %d\n\tproto_type: 0x%04x\n\thw_len: %d\n\tproto_len: "
      "%d\n\ttype: %d\n",
      net_ntohs(arp_pkt->hw_type), net_ntohs(arp_pkt->proto_type),
      arp_pkt->hw_addr_size, arp_pkt->proto_addr_size, op_code);

  switch (op_code) {
    case ARP_OP_REQUEST:
      plat_printf("\top: request\n");
      break;
    case ARP_OP_REPLY:
      plat_printf("\top: reply\n");
      break;
    default:
      plat_printf("\top: unknown\n");
      break;
  }

  ipaddr_t ipaddr;
  ipaddr.type = IPADDR_V4;

  plat_memcpy(ipaddr.addr_bytes, arp_pkt->sender_proto_addr, IPV4_ADDR_SIZE);
  netif_dum_ip("\tsender ip: ", &ipaddr);
  netif_dum_hwaddr("\tsender mac: ", arp_pkt->sender_hw_addr, ETHER_MAC_SIZE);

  plat_printf("\n");

  plat_memcpy(ipaddr.addr_bytes, arp_pkt->target_proto_addr, IPV4_ADDR_SIZE);
  netif_dum_ip("\ttarget ip: ", &ipaddr);
  netif_dum_hwaddr("\ttarget mac: ", arp_pkt->target_hw_addr, ETHER_MAC_SIZE);

  plat_printf("\n---------------------------------------------\n");
}

#else
#define arp_cache_entry_display(entry)
#define arp_tbl_display()
#define arp_pkt_display(arp_pkt)
#endif

/**
 * @brief 初始化arp缓存表
 *
 * @return net_err_t
 */
static net_err_t arp_cache_init(void) {
  net_err_t err = NET_ERR_OK;
  // 初始化arp缓存表链表
  nlist_init(&cache_tbl_list);

  // 初始化arp缓存表内存块管理对象
  // 只由arp模块使用，并且只由一个工作线程管理，不需要锁保护
  err = mblock_init(&cache_tbl_mblock, cache_tbl, sizeof(arp_entry_t),
                    ARP_CACHE_SIZE, NLOCKER_NONE);
  Net_Err_Check(err);

  return NET_ERR_OK;
}

/**
 * @brief 初始化arp模块
 *
 * @return net_err_t
 */
net_err_t arp_module_init(void) {
  dbg_info(DBG_ARP, "init arp module....");

  net_err_t err = NET_ERR_OK;

  // 初始化arp缓存表
  err = arp_cache_init();
  if (err != NET_ERR_OK) {
    dbg_error(DBG_ARP, "arp module init error: arp cache init failed.");
    return err;
  }

  dbg_info(DBG_ARP, "init arp module ok.");
  return NET_ERR_OK;
}

/**
 * @brief 构造arp请求包, 并将请求包数据填充到buf中
 *
 * @param netif
 * @param dest
 * @return net_err_t
 */
net_err_t arp_make_request(netif_t *netif, const ipaddr_t *dest) {
  net_err_t err = NET_ERR_OK;

  // 构造ARP请求包
  pktbuf_t *arp_buf = pktbuf_alloc(sizeof(arp_pkt_t));  //!!! 分配数据包
  if (arp_buf == (pktbuf_t *)0) {
    dbg_error(DBG_ETHER, "arp request error: alloc pktbuf failed.");
    return NET_ERR_MEM;
  }

  // 设置buf头部在内存上的连续性
  pktbuf_set_cont(arp_buf, sizeof(arp_pkt_t));  // 必定成功

  // 构造arp请求包
  arp_pkt_t *arp_pkt = (arp_pkt_t *)pktbuf_data_ptr(arp_buf);
  arp_pkt->hw_type = net_htons(ARP_HW_ETHER);
  arp_pkt->proto_type = net_htons(NET_PROTOCOL_IPV4);
  arp_pkt->hw_addr_size = ETHER_MAC_SIZE;
  arp_pkt->proto_addr_size = IPV4_ADDR_SIZE;
  arp_pkt->op_code = net_htons(ARP_OP_REQUEST);
  plat_memcpy(arp_pkt->sender_hw_addr, netif->hwaddr.addr, ETHER_MAC_SIZE);
  plat_memcpy(arp_pkt->sender_proto_addr, netif->ipaddr.addr_bytes,
              IPV4_ADDR_SIZE);
  plat_memset(arp_pkt->target_hw_addr, 0, ETHER_MAC_SIZE);
  plat_memcpy(arp_pkt->target_proto_addr, dest->addr_bytes, IPV4_ADDR_SIZE);

  arp_pkt_display(arp_pkt);

  // 发送ARP请求包
  err = ether_raw_send(netif, NET_PROTOCOL_ARP, ether_broadcast_addr(),
                       arp_buf);  //!!! 数据包传递
  if (err != NET_ERR_OK) {
    dbg_error(DBG_ETHER, "arp request error: send arp request failed.");
    //!!! 释放数据包
    pktbuf_free(arp_buf);

    return err;
  }

  return NET_ERR_OK;
}

/**
 * @brief 发送一个arp应答包
 *
 * @param netif
 * @param buf
 * @return net_err_t
 */
net_err_t arp_make_reply(netif_t *netif, pktbuf_t *buf) {
  arp_pkt_t *arp_pkt = (arp_pkt_t *)pktbuf_data_ptr(buf);

  arp_pkt->op_code = net_htons(ARP_OP_REPLY);
  plat_memcpy(arp_pkt->target_hw_addr, arp_pkt->sender_hw_addr, ETHER_MAC_SIZE);
  plat_memcpy(arp_pkt->target_proto_addr, arp_pkt->sender_proto_addr,
              IPV4_ADDR_SIZE);
  plat_memcpy(arp_pkt->sender_hw_addr, netif->hwaddr.addr, ETHER_MAC_SIZE);
  plat_memcpy(arp_pkt->sender_proto_addr, netif->ipaddr.addr_bytes,
              IPV4_ADDR_SIZE);

  arp_pkt_display(arp_pkt);

  net_err_t err =
      ether_raw_send(netif, NET_PROTOCOL_ARP, arp_pkt->target_hw_addr, buf);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_ETHER, "arp reply error: send arp reply failed.");
    return err;
  }

  return NET_ERR_OK;
}

/**
 * @brief 发送免费ARP包，免费arp(gratuitous arp)的作用:
 *  1. 检测本网络接口的IP地址在局域网中是否已被使用
 *  2. 若IP未被使用，通知局域网中的其他设备，本机已经使用了该IP地址, 、
 *     若其它设备之前向该ip地址发送过arp请求，那么它们会同时更新自己的arp缓存表，使该ip地址映射到本机的mac地址
 *  3. 若IP已被使用，则警告网络管理员，有IP地址冲突
 *
 * @param netif
 * @param dest
 * @return net_err_t
 */
net_err_t arp_make_gratuitous(netif_t *netif) {
  dbg_info(DBG_ARP, "send an gratuitous arp pkt...");

  return arp_make_request(netif, &netif->ipaddr);
}

/**
 * @brief 检测arp数据包是否合法
 *
 * @param arp_pkt
 * @param size
 * @param netif
 * @return net_err_t
 */
static net_err_t arp_pkt_check(arp_pkt_t *arp_pkt, uint16_t size,
                               netif_t *netif) {
  // 包长度检测
  if (size < sizeof(arp_pkt_t)) {
    dbg_warning(DBG_ARP, "arp pkt check warning: arp pkt size too small.");
    return NET_ERR_SIZE;
  }

  // 包字段检测
  if ((arp_pkt->hw_type != net_htons(ARP_HW_ETHER)) ||
      (arp_pkt->hw_addr_size != ETHER_MAC_SIZE) ||
      (arp_pkt->proto_type != net_htons(NET_PROTOCOL_IPV4) ||
       (arp_pkt->proto_addr_size != IPV4_ADDR_SIZE))) {
    dbg_warning(DBG_ARP, "arp pkt check warning: arp pkt field error.");
    return NET_ERR_PROTO;
  }

  // 操作码检测
  uint16_t op_code = net_ntohs(arp_pkt->op_code);
  if ((op_code != ARP_OP_REQUEST) && (op_code != ARP_OP_REPLY)) {
    dbg_warning(DBG_ARP, "arp pkt check warning: unknown op code.");
    return NET_ERR_PROTO;
  }

  return NET_ERR_OK;
}

/**
 * @brief 对从链路层接收到的arp数据包进行处理
 *
 * @param netif
 * @param buf
 * @return net_err_t
 */
net_err_t arp_recv(netif_t *netif, pktbuf_t *buf) {
  dbg_info(DBG_ARP, "recv an arp pkt...");

  pktbuf_check_buf(buf);
  net_err_t err = NET_ERR_OK;

  // 设置buf头部在内存上的连续性
  err = pktbuf_set_cont(buf, sizeof(arp_pkt_t));
  if (err != NET_ERR_OK) {
    dbg_error(DBG_ARP, "arp recv error: set buf cont failed.");
    return err;
  }

  // 获取arp数据包结构
  arp_pkt_t *arp_pkt = (arp_pkt_t *)pktbuf_data_ptr(buf);

  // 检测arp数据包是否合法
  err = arp_pkt_check(arp_pkt, pktbuf_total_size(buf), netif);
  if (err != NET_ERR_OK) {
    dbg_warning(DBG_ARP, "arp recv error: arp pkt check failed.");
    return err;
  }

  arp_pkt_display(arp_pkt);

  // TODO 处理arp数据包
  if (net_ntohs(arp_pkt->op_code) == ARP_OP_REQUEST) {  // arp请求包
    dbg_info(DBG_ARP, "recv an arp request pkt...");

    return arp_make_reply(netif, buf);
  }

  return NET_ERR_OK;
}