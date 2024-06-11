/**
 * @file ether.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 链路层：以太网协议层接口实现
 * @version 0.1
 * @date 2024-05-26
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "ether.h"

#include "arp.h"
#include "dbg.h"
#include "netif.h"
#include "pktbuf.h"
#include "protocol.h"
#include "tools.h"

#if DBG_DISP_ENABLED(DBG_ETHER)

/**
 * @brief 打印以太网帧信息
 *
 * @param msg
 * @param pkt
 * @param total_size
 */
static void display_ether_pkt(char *msg, ether_pkt_t *pkt, int total_size) {
  // 获取以太网帧头部
  ether_hdr_t *hdr = &(pkt->hdr);
  plat_printf("------------------------------ %s -----------------------------\n", msg);
  plat_printf("len: %d bytes", total_size);

  netif_dum_hwaddr("\tdest: ", hdr->dest, ETHER_MAC_SIZE);
  netif_dum_hwaddr("\tsrc: ", hdr->src, ETHER_MAC_SIZE);

  uint16_t protocol_type = net_ntohs(hdr->protocol_type);
  plat_printf(
      "\ttype: 0x%04x",
      protocol_type);  // 协议类型为u16，需要将网络字节序转换为主机字节序

  switch (protocol_type) {
    case NET_PROTOCOL_ARP:
      plat_printf("\tarp\n");
      break;
    case NET_PROTOCOL_IPV4:
      plat_printf("\tipv4\n");
      break;
    default:
      plat_printf("\tunknown\n");
      break;
  }

  plat_printf("-----------------------------------------------------------------------\n");
}

#else

#define display_ether_pkt(msg, pkt, total_size)

#endif

/**
 * @brief 检查以太网帧的正确性
 *
 * @param pkt
 * @param total_size
 * @return net_err_t
 */
static net_err_t ether_pkt_check(ether_pkt_t *pkt, int total_size) {
  if (total_size > sizeof(ether_hdr_t) + ETHER_MTU) {  // 以太网帧大小超过最大值
    dbg_warning(DBG_ETHER, "ether pkt size is too large.");
    return NET_ERR_SIZE;
  }

  //! 以太网帧最小值应该为 MAC包头(14B) + 最小负载(46B) = 60B
  //! 但是PCAP库会自动将有效载荷中的填充字节去掉，所以导致接收到的以太网帧的有效载荷
  //! 可能小于46B，所以这里只判断以太网帧的大小是否小于MAC包头的大小，
  //! 不对有效载荷大小进行判断
  if (total_size < sizeof(ether_hdr_t)) {  // 以太网帧大小小于最小值
    dbg_warning(DBG_ETHER, "ether pkt size less than ether header size.");
    return NET_ERR_SIZE;
  }
  return NET_ERR_OK;
}

/**
 * @brief 进行以太网协议层相关的初始化
 * @param netif
 * @return net_err_t
 */
static net_err_t ether_open(netif_t *netif) {
  // 发送免费ARP请求
  return arp_make_gratuitous(netif);
}

/**
 * @brief 完成以太网协议层相关的清理
 *
 * @param netif
 */
static void ether_close(netif_t *netif) { return; }

/**
 * @brief 完成以太网协议层对接收数据包的处理
 *
 * @param netif
 * @param pktbuf
 * @return net_err_t
 */
static net_err_t ether_recv(netif_t *netif, pktbuf_t *buf) {
  dbg_info(DBG_ETHER, "link layer ether recving....");

  net_err_t err = NET_ERR_OK;

  // 获取以太网帧
  pktbuf_set_cont(buf, sizeof(ether_hdr_t));  // 确保帧头部在内存上的连续性
  ether_pkt_t *pkt = (ether_pkt_t *)pktbuf_data_ptr(buf);

  if ((err = ether_pkt_check(pkt, pktbuf_total_size(buf))) != NET_ERR_OK) {
    dbg_error(DBG_ETHER, "link layer ether recv failed.");
    return err;
  }

  // 打印以太网帧
  display_ether_pkt("ether recv", pkt, pktbuf_total_size(buf));

  // 判断协议类型
  switch (net_ntohs(pkt->hdr.protocol_type)) {
    case NET_PROTOCOL_ARP: {
      // ARP协议
      err = pktbuf_header_remove(buf, sizeof(ether_hdr_t));  // 移除以太网帧头部
      Net_Err_Check(err);
      err = arp_recv(netif, buf);
      if (err != NET_ERR_OK) {
        dbg_error(DBG_ETHER, "link layer ether recv error: arp recv failed.");
        return err;
      }
    } break;
    case NET_PROTOCOL_IPV4: {
      // IPv4协议
      // err = netif_recvq_put(netif, buf, -1);
    } break;
    default:
      dbg_warning(DBG_ETHER,
                  "link layer ether recv warning: unknown protocol.");
      break;
  }

  dbg_info(DBG_ETHER, "link layer ether recv ok.");

  return NET_ERR_OK;
}

/**
 * @brief 向以太网协议层发送数据包
 *
 * @param netif
 * @param ipdest
 * @param buf
 * @return net_err_t
 */
static net_err_t ether_send(netif_t *netif, const ipaddr_t *ipdest, pktbuf_t *buf) {
  if (ipaddr_is_equal(ipdest, &netif->ipaddr)) {
    // 目的IP地址与本地IP地址相同，则无需进行arp查询，直接发送
    return ether_raw_send(netif, NET_PROTOCOL_IPV4, netif->hwaddr.addr, buf); //!!! 数据包传递
  }

  return arp_send(netif, ipdest, buf);  //!!! 数据包传递
}

/**
 * @brief 初始化以太网协议层
 *
 * @return net_err_t
 */
net_err_t ether_module_init(void) {
  static const link_layer_t ether_ops = {
      // 将接口暴露给外部
      .type = NETIF_TYPE_ETHER, .open = ether_open, .close = ether_close,
      .recv = ether_recv,       .send = ether_send,
  };

  dbg_info(DBG_ETHER, "init ether....");

  // 将以太网协议层回调接口注册到网络接口模块的链路层回调接口中
  net_err_t err = netif_layer_register(&ether_ops);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_ETHER, "register ether failed.");
    return err;
  }

  dbg_info(DBG_ETHER, "init ether ok.");
}

/**
 * @brief 获取以太网广播地址
 * 广播：FF-FF-FF-FF-FF-FF
 * 多播(组播)
 * 单播
 *
 *
 * @return const uint8_t*
 */
const uint8_t *ether_broadcast_addr(void) {
  static const uint8_t broadcast_addr[ETHER_MAC_SIZE] = {0xFF, 0xFF, 0xFF,
                                                         0xFF, 0xFF, 0xFF};

  return broadcast_addr;
}

/**
 * @brief 封装以太网帧并发送
 *
 * @param netif
 * @param protocol
 * @param dest_mac_addr
 * @param buf
 * @return net_err_t
 */
net_err_t ether_raw_send(netif_t *netif, protocol_type_t protocol,
                         const uint8_t *dest_mac_addr, pktbuf_t *buf) {
  net_err_t err = NET_ERR_OK;
  int total_size = pktbuf_total_size(buf);

  if (total_size < ETHER_DATA_MIN) {
    dbg_info(DBG_ETHER, "resize ether data from %d to %d.", total_size,
             ETHER_DATA_MIN);

    // 重新调整数据包大小到以太网数据载荷最小值
    err = pktbuf_resize(buf, ETHER_DATA_MIN);
    Net_Err_Check(err);

    pktbuf_seek(buf, total_size);  // 访问位置移动到补充区域的起始位置
    pktbuf_fill(buf, 0, ETHER_DATA_MIN - total_size);  // 在补充的区域填充0
  }

  // 添加以太网包头，确保以太网包头在内存上的连续性
  err = pktbuf_header_add(buf, sizeof(ether_hdr_t), PKTBUF_ADD_HEADER_CONT);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_ETHER, "add ether header failed.");
    return err;
  }

  // 填写以太网包头信息
  ether_pkt_t *pkt = (ether_pkt_t *)pktbuf_data_ptr(buf);
  plat_memcpy(pkt->hdr.dest, dest_mac_addr, ETHER_MAC_SIZE);  // 目的mac地址
  plat_memcpy(pkt->hdr.src, netif->hwaddr.addr, ETHER_MAC_SIZE);  // 源mac地址
  pkt->hdr.protocol_type = net_htons(protocol);  // 帧协议类型

  display_ether_pkt("ehter send pkt: ", pkt, pktbuf_total_size(buf));

  if (plat_memcmp(netif->hwaddr.addr, pkt->hdr.dest, ETHER_MAC_SIZE) == 0) {
    // 目的mac地址与本地mac地址相同，直接放入接收队列
    return netif_recvq_put(netif, buf, -1);
  } else {
    // 将数据包放入发送队列
    err = netif_sendq_put(netif, buf, -1);
    if (err != NET_ERR_OK) {
      dbg_warning(DBG_ETHER, "send ether pkt failed.");
      return err;
    }
    return netif->ops->send(netif);  // 通过网卡发送数据包
  }
}
