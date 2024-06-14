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
#include "ipaddr.h"
#include "mblock.h"
#include "net_cfg.h"
#include "net_err.h"
#include "nlocker.h"
#include "timer.h"
#include "tools.h"

static arp_entry_t cache_tbl[ARP_CACHE_SIZE];  // arp缓存表
static mblock_t cache_tbl_mblock;  // arp缓存表内存块管理对象
static nlist_t cache_entry_list;   // 管理已分配的arp缓存表
static net_timer_t cache_timer;    // arp缓存表定时器

#if DBG_DISP_ENABLED(DBG_ARP)

/**
 * @brief 打印arp缓存表项
 *
 * @param entry
 */
static void arp_entry_display(arp_entry_t *entry) {
  ipaddr_t ipaddr;
  ipaddr.type = IPADDR_V4;
  plat_memcpy(ipaddr.addr_bytes, entry->ipaddr, IPV4_ADDR_SIZE);
  netif_dum_ip("ip: ", &ipaddr);
  netif_dum_hwaddr("\tmac: ", entry->hwaddr, ETHER_MAC_SIZE);

  plat_printf("\ttmo: %d, retry: %d, state: %s  pktbuf_cnt: %d\n", entry->tmo,
              entry->retry,
              entry->state == NET_ARP_RESOLVED ? "Resolved" : "Waitting",
              nlist_count(&entry->buf_list));
}

/**
 * @brief 遍历打印已使用的arp缓存表项
 *
 */
static void arp_tbl_display(void) {
  nlist_node_t *node;
  arp_entry_t *entry;

  plat_printf("---------------- arp cache table ----------------\n");
  nlist_for_each(node, &cache_entry_list) {
    entry = nlist_entry(node, arp_entry_t, node);

    arp_entry_display(entry);
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
 * @brief 清空arp缓存表项所缓存的数据包
 *
 * @param entry
 */
static void arp_entry_clear(arp_entry_t *entry) {
  // 释放缓存的数据包
  nlist_node_t *node;
  pktbuf_t *buf;
  while ((node = nlist_remove_first(&entry->buf_list)) != (nlist_node_t *)0) {
    buf = nlist_entry(node, pktbuf_t, node);

    //!!! 释放数据包
    pktbuf_free(buf);
  }
}

/**
 * @brief 释放arp缓存表项
 *
 * @param entry
 */
static void arp_entry_free(arp_entry_t *entry) {
  // 清空缓存的数据包
  arp_entry_clear(entry);

  // 从已分配的arp缓存表链表中移除
  nlist_remove(&cache_entry_list, &entry->node);

  // 释放该arp缓存表项
  mblock_free(&cache_tbl_mblock, entry);
}


/**
 * @brief arp缓存表定时器回调函数，用于扫描并处理arp缓存表项超时
 *
 * @param timer
 * @param arg
 */
static void arp_cache_tmo(net_timer_t *timer, void *arg) {
  nlist_node_t *node;
  int flag = 0;  // 标记是否有超时的表项

  // 遍历arp缓存表, 处理超时的arp缓存表项
  nlist_for_each(node, &cache_entry_list) {
    arp_entry_t *entry = nlist_entry(node, arp_entry_t, node);

    if (--entry->tmo > 0) {  // 未超时
      continue;
    }

    flag = 1;  // 标记有超时的表项

    // 超时处理
    switch (entry->state) {
      case NET_ARP_RESOLVED: {  // 当前arp缓存表项已解析，重新获取目标mac地址以更新
        dbg_info(DBG_ARP, "arp cache tmo info: arp entry resolved tmo.");
        arp_entry_display(entry);

        // 重载表项的tmo和retry，并进入等待状态
        entry->tmo = ARP_ENTRY_WAITING_TMO;
        entry->retry = ARP_ENTRY_RETRY_CNT;
        entry->state = NET_ARP_WAITING;

        // 发送arp请求包
        arp_make_request(entry->netif, entry->ipaddr);
      } break;
      case NET_ARP_WAITING: {  // 当前缓存表项未解析，尝试重新发送arp请求包
        dbg_info(DBG_ARP, "arp cache tmo info: arp entry waiting tmo.");
        arp_entry_display(entry);

        if (entry->retry-- > 0) {  // 未达到最大重试次数，可再次发送arp请求包
          // 重载表项的tmo，并继续等待
          entry->tmo = ARP_ENTRY_WAITING_TMO;

          // 发送arp请求包
          arp_make_request(entry->netif, entry->ipaddr);
        } else {  // 达到最大重试次数，释放该表项
          dbg_warning(DBG_ARP, "arp cache tmo warning: arp entry retry max.");
          arp_entry_free(entry);
        }

      } break;
      default: {  // 未知状态,释放该表项
        dbg_error(DBG_ARP, "arp cache tmo error: unknown arp entry state.");
        arp_entry_free(entry);
      } break;
    }
  }

  if (flag) {  // 有超时的表项，打印arp缓存表
    arp_tbl_display();
  }
}

/**
 * @brief 初始化arp缓存表
 *
 * @return net_err_t
 */
static net_err_t arp_cache_init(void) {
  net_err_t err = NET_ERR_OK;
  // 初始化arp缓存表链表
  nlist_init(&cache_entry_list);

  // 初始化arp缓存表内存块管理对象
  // 只由arp模块使用，并且只由一个工作线程管理，不需要锁保护
  err = mblock_init(&cache_tbl_mblock, cache_tbl, sizeof(arp_entry_t),
                    ARP_CACHE_SIZE, NLOCKER_NONE);
  Net_Err_Check(err);

  return NET_ERR_OK;
}


/**
 * @brief 分配一个arp缓存表项
 *
 * @return arp_entry_t*
 */
static arp_entry_t *arp_entry_alloc(int force) {
  // 从内存块管理对象中分配一个arp缓存表项
  arp_entry_t *entry = (arp_entry_t *)mblock_alloc(&cache_tbl_mblock, -1);
  if (!entry && force) {  // 已无空闲内存块，强制释放一个
    // 重新使用最早使用的arp缓存表项
    nlist_node_t *node = nlist_remove_last(&cache_entry_list);
    entry = nlist_entry(node, arp_entry_t, node);
    arp_entry_clear(entry);  // 清空缓存的数据包
  }

  if (entry) {
    // 初始化该表项
    entry->state = NET_ARP_FREE;
    nlist_node_init(&entry->node);
    nlist_init(&entry->buf_list);
  }

  return entry;
}

/**
 * @brief 根据ip地址，在cache_entry_list中查询arp缓存表项
 *
 * @param ipaddr
 * @return arp_entry_t*
 */
arp_entry_t *arp_entry_find(const uint8_t *ipaddr_bytes) {
  nlist_node_t *node;
  arp_entry_t *entry;

  nlist_for_each(node, &cache_entry_list) {
    entry = nlist_entry(node, arp_entry_t, node);

    if (plat_memcmp(entry->ipaddr, ipaddr_bytes, IPV4_ADDR_SIZE) == 0) {
      // 若表项不在链表头，则将其移动到链表头，以便下次快速查找
      if (&entry->node != nlist_first(&cache_entry_list)) {
        nlist_remove(&cache_entry_list, &entry->node);
        nlist_insert_first(&cache_entry_list, &entry->node);
      }

      return entry;
    }
  }

  return (arp_entry_t *)0;
}

/**
 * @brief 设置arp缓存表项的ip地址、硬件地址和状态
 *
 * @param entry
 * @param ipaddr
 * @param hwaddr
 * @param state
 */
static void arp_entry_set(arp_entry_t *entry, const uint8_t *ipaddr_bytes,
                          const uint8_t *hwaddr_bytes, netif_t *netif,
                          int state) {
  plat_memcpy(entry->ipaddr, ipaddr_bytes, IPV4_ADDR_SIZE);
  plat_memcpy(entry->hwaddr, hwaddr_bytes, ETHER_MAC_SIZE);
  entry->netif = netif;
  entry->state = state;
  entry->retry = ARP_ENTRY_RETRY_CNT;

  // 根据状态设置表项的超时时间
  if (state == NET_ARP_RESOLVED) {
    entry->tmo = ARP_ENTRY_RESOLVED_TMO;
  } else {
    entry->tmo = ARP_ENTRY_WAITING_TMO;
  }
}

/**
 * @brief 将arp缓存表项的数据包全部发送出去
 *
 * @param entry
 * @return net_err_t
 */
static net_err_t arp_entry_send_all(arp_entry_t *entry) {
  net_err_t err = NET_ERR_OK;

  nlist_node_t *node;
  pktbuf_t *buf;
  while ((node = nlist_remove_first(&entry->buf_list)) != (nlist_node_t *)0) {
    //!!! 获取数据包
    buf = nlist_entry(node, pktbuf_t, node);

    // 发送数据包
    err = ether_raw_send(entry->netif, NET_PROTOCOL_IPV4, entry->hwaddr,
                         buf);  //!!! 数据包传递
    if (err != NET_ERR_OK) {
      dbg_warning(DBG_ARP, "arp entry send all warning: buf loss.");
      pktbuf_free(buf);  //!!! 释放数据包
    }
  }

  return NET_ERR_OK;
}

/**
 * @brief 将arp缓存表项插入到cache_entry_list中
 *
 * @param entry
 * @param ipaddr
 * @param hwaddr
 * @param force 若内存块已满，是否强制释放一个，以便分配新的内存块(1: 强制释放,
 * 0: 不强制释放, 放弃插入)
 * @return net_err_t
 */
static net_err_t arp_entry_insert(netif_t *netif, const uint8_t *ipaddr_bytes,
                                  uint8_t *hwaddr, int force) {
  // 空的ip地址不进行缓存
  if (*(uint32_t *)ipaddr_bytes == 0) {
    return NET_ERR_NOSUPPORT;
  }

  net_err_t err = NET_ERR_OK;

  // 查找ipaddr对应的arp缓存表项是否已存在
  arp_entry_t *entry = arp_entry_find(ipaddr_bytes);
  if (entry) {  // 表项已存在，只需更新即可
    // 更新表项
    arp_entry_set(entry, ipaddr_bytes, hwaddr, netif, NET_ARP_RESOLVED);

    // 将表项缓存的数据包发送出去
    err = arp_entry_send_all(entry);
    if (err != NET_ERR_OK) {
      dbg_error(DBG_ARP, "arp entry update error: send all buf failed.");
      return err;
    }

  } else {  // 表项不存在，需要分配一个
    entry = arp_entry_alloc(force);
    if (!entry) {  // 分配失败
      dbg_warning(DBG_ARP,
                  "arp entry insert waring: alloc arp cache entry failed.");
      return NET_ERR_MEM;
    }
    // 设置表项
    arp_entry_set(entry, ipaddr_bytes, hwaddr, netif, NET_ARP_RESOLVED);
    // 将表项插入到链表头, 以便下次快速查找
    nlist_insert_first(&cache_entry_list, &entry->node);
  }

  arp_tbl_display();

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

  // 初始化arp缓存表定时器
  err =
      net_timer_add(&cache_timer, "arp cache timer", arp_cache_tmo, (void *)0,
                    ARP_CACHE_TMO * 1000, NET_TIMER_ACTIVE | NET_TIMER_RELOAD);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_ARP, "arp module init error: arp cache timer init failed.");
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
net_err_t arp_make_request(netif_t *netif, const uint8_t *dest_ipaddr_bytes) {
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
  plat_memcpy(arp_pkt->target_proto_addr, dest_ipaddr_bytes, IPV4_ADDR_SIZE);

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
 * @param buf 需要响应的arp请求包
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

  net_err_t err = ether_raw_send(
      netif, NET_PROTOCOL_ARP, arp_pkt->target_hw_addr, buf);  //!!! 数据包传递
  if (err != NET_ERR_OK) {
    dbg_error(DBG_ETHER, "arp reply error: send arp reply failed.");
    return err;
  }

  return NET_ERR_OK;
}

/**
 * @brief 发送一个arp包警告地址冲突
 * 还可以更新局域网其它设备的arp缓存表
 * @param netif
 * @param buf
 * @return net_err_t
 */
static net_err_t arp_make_conflict(netif_t *netif, pktbuf_t *buf) {
  arp_pkt_t *arp_pkt = (arp_pkt_t *)pktbuf_data_ptr(buf);

  arp_pkt->op_code = net_htons(ARP_OP_REPLY);

  // 将目标mac地址和目标ip地址设置为0，以通告所有设备，发生了ip地址冲突，且当前ip地址已被本机使用
  plat_memset(arp_pkt->target_hw_addr, 0, ETHER_MAC_SIZE);
  plat_memset(arp_pkt->target_proto_addr, 0, IPV4_ADDR_SIZE);
  plat_memcpy(arp_pkt->sender_hw_addr, netif->hwaddr.addr, ETHER_MAC_SIZE);
  plat_memcpy(arp_pkt->sender_proto_addr, netif->ipaddr.addr_bytes,
              IPV4_ADDR_SIZE);

  arp_pkt_display(arp_pkt);

  net_err_t err = ether_raw_send(
      netif, NET_PROTOCOL_ARP, arp_pkt->target_hw_addr, buf);  //!!! 数据包传递
  if (err != NET_ERR_OK) {
    dbg_error(DBG_ETHER, "arp reply error: send arp reply failed.");
    return err;
  }
}

/**
 * @brief 发送一个arp探测包，用于检测目标ip地址是否已被使用
 *
 * @param netif
 * @param dest_ipaddr_bytes
 * @return net_err_t
 */
net_err_t arp_make_probe(netif_t *netif) {
  net_err_t err = NET_ERR_OK;

  dbg_info(DBG_ARP, "send an arp probe pkt...");

  // 构造ARP请求包
  pktbuf_t *arp_buf = pktbuf_alloc(sizeof(arp_pkt_t));  //!!! 分配数据包
  if (arp_buf == (pktbuf_t *)0) {
    dbg_error(DBG_ETHER, "arp probe error: alloc pktbuf failed.");
    return NET_ERR_MEM;
  }

  // 设置buf头部在内存上的连续性
  pktbuf_set_cont(arp_buf, sizeof(arp_pkt_t));  // 必定成功

  // 构造arp探测
  arp_pkt_t *arp_pkt = (arp_pkt_t *)pktbuf_data_ptr(arp_buf);
  arp_pkt->hw_type = net_htons(ARP_HW_ETHER);
  arp_pkt->proto_type = net_htons(NET_PROTOCOL_IPV4);
  arp_pkt->hw_addr_size = ETHER_MAC_SIZE;
  arp_pkt->proto_addr_size = IPV4_ADDR_SIZE;
  arp_pkt->op_code = net_htons(ARP_OP_REQUEST);  // 探测包的类型为请求包

  // 将发送方的ip地址和接收方的mac地址设置为0，以产生一个arp探测包
  plat_memcpy(arp_pkt->sender_hw_addr, netif->hwaddr.addr, ETHER_MAC_SIZE);
  plat_memset(arp_pkt->sender_proto_addr, 0, IPV4_ADDR_SIZE);
  plat_memset(arp_pkt->target_hw_addr, 0, ETHER_MAC_SIZE);
  plat_memcpy(arp_pkt->target_proto_addr, netif->ipaddr.addr_bytes,
              IPV4_ADDR_SIZE);

  arp_pkt_display(arp_pkt);

  // 发送ARP探测包
  err = ether_raw_send(netif, NET_PROTOCOL_ARP, ether_broadcast_addr(),
                       arp_buf);  //!!! 数据包传递
  if (err != NET_ERR_OK) {
    dbg_error(DBG_ETHER, "arp probe error: send arp request failed.");
    //!!! 释放数据包
    pktbuf_free(arp_buf);
    return err;
  }

  return NET_ERR_OK;
}

/**
 * @brief 发送免费ARP包(arp通告包)，免费arp(gratuitous arp)的作用:
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

  return arp_make_request(netif, ipaddr_get_bytes(&netif->ipaddr));
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
 * !!! 该函数可释放数据包 和 传递数据包
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

  // 检测arp数据包发送方的ip地址是否与本机的ip地址相同
  ipaddr_t ipaddr;
  ipaddr_from_bytes(&ipaddr, arp_pkt->sender_proto_addr);
  if (ipaddr_is_equal(&ipaddr, &netif->ipaddr)) {
    dbg_warning(DBG_ARP,
                "arp recv warning: send ipaddr is same as local ipaddr.");
    // 根据当前网卡的状态，决定是否发送arp冲突包
    if (netif->state == NETIF_STATE_ACVTIVE) {  // 网卡已激活, 发送arp冲突包
      return arp_make_conflict(netif, buf);  //!!! 数据包传递
    } else {  // 网卡未激活，设置网卡状态为ip地址冲突
      netif_set_ipconflict(netif);
    }
    return NET_ERR_CONFLICT;
  }

  // 检测arp数据包是否是发给本机的
  ipaddr_from_bytes(&ipaddr, arp_pkt->target_proto_addr);
  if (ipaddr_is_equal(&ipaddr,
                      &netif->ipaddr)) {  // 是发给本机的, 进行数据包处理
    switch (net_ntohs(arp_pkt->op_code)) {
      case ARP_OP_REQUEST: {  // arp请求包
        dbg_info(DBG_ARP, "recv an arp request pkt...");
        // 直接从请求包中获取目的ip地址对应的mac地址
        err = arp_entry_insert(netif, arp_pkt->sender_proto_addr,
                               arp_pkt->sender_hw_addr, 1);
        if (err != NET_ERR_OK) {
          dbg_warning(DBG_ARP,
                      "arp recv warning: insert arp cache entry failed.");
        }

        return arp_make_reply(netif, buf);  //!!! 传递数据包
      } break;
      case ARP_OP_REPLY: {  // arp响应包
        dbg_info(DBG_ARP, "recv an arp reply pkt...");

        // 将arp缓存表项插入到cache_entry_list中
        err = arp_entry_insert(netif, arp_pkt->sender_proto_addr,
                               arp_pkt->sender_hw_addr, 1);
        if (err != NET_ERR_OK) {
          dbg_warning(DBG_ARP,
                      "arp recv warning: insert arp cache entry failed.");
          return err;
        }
      }
    }
  } else {  // 不是发给本机的，可尝试缓存对方主机的ip地址和mac地址
    dbg_warning(DBG_ARP, "arp recv warning: arp pkt not for me.");
    arp_entry_insert(netif, arp_pkt->sender_proto_addr, arp_pkt->sender_hw_addr,
                     0);
  }

  // 当前数据包已处理完毕
  pktbuf_free(buf);  //!!! 释放数据包

  return NET_ERR_OK;
}

/**
 * @brief 通过arp协议获取目的ip地址对应的mac地址
 * 并将链路层传来的数据包发送出去
 *
 * @param netif
 * @param ipdest
 * @param buf
 * @return net_err_t
 */
net_err_t arp_send(netif_t *netif, const uint8_t *dest_ipaddr_bytes,
                   pktbuf_t *buf) {
  net_err_t err = NET_ERR_OK;

  // 查找ip地址对应的arp缓存表项
  arp_entry_t *entry = arp_entry_find(dest_ipaddr_bytes);
  if (entry) {                               // 查询成功
    if (entry->state == NET_ARP_RESOLVED) {  // 该表项已解析，可获取mac地址
      // 发送数据包
      return ether_raw_send(netif, NET_PROTOCOL_IPV4, entry->hwaddr,
                            buf);  //!!! 数据包传递
    }

    // 该表项未解析，将数据包缓存到表项中
    if (nlist_count(&entry->buf_list) <= ARP_WAIT_PKT_MAXCNT) {
      // 表项缓存数据包数量不能超过最大值，防止大量数据包缓存导致内存耗尽，从而无法为其它数据包分配内存
      nlist_insert_last(&entry->buf_list, &buf->node);  //!!! 数据包转交
      dbg_info(DBG_ARP, "arp send info: buf cached.");
      return NET_ERR_OK;
    } else {
      dbg_warning(DBG_ARP, "arp send warning: loss buf, arp cache full.");
      return NET_ERR_FULL;
    }
  } else {
    // 该表项不存在，分配一个新的表项
    entry = arp_entry_alloc(1);
    if (!entry) {
      dbg_error(DBG_ARP, "arp send error: alloc arp cache entry failed.");
      return NET_ERR_MEM;
    }

    // 设置表项
    arp_entry_set(entry, dest_ipaddr_bytes, ether_broadcast_addr(), netif,
                  NET_ARP_WAITING);

    // 将数据包缓存到表项中
    nlist_insert_last(&entry->buf_list, &buf->node);  //!!! 数据包转交

    // 将表项插入到链表头, 以便下次快速查找
    nlist_insert_first(&cache_entry_list, &entry->node);

    // 发送arp请求包
    // 不能因为arp请求失败而返回错误， 当前数据包已交给arp缓存表项管理
    // 若返回失败，会导致上层调用者释放数据包，导致arp缓存表项中的数据包失效
    arp_make_request(netif, dest_ipaddr_bytes);
  }

  return NET_ERR_OK;
}