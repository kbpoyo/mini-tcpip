/**
 * @file ipv4.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief ipv4协议模块
 * @version 0.1
 * @date 2024-06-15
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "ipv4.h"

#include "dbg.h"
#include "ether.h"
#include "tools.h"
#include "protocol.h"
/**
 * @brief 初始化ipv4协议模块
 *
 * @return net_err_t
 */
net_err_t ipv4_module_init(void) {
  dbg_info(DBG_IPV4, "init ipv4 module....");

  dbg_info(DBG_IPV4, "init ipv4 module ok.");
  return NET_ERR_OK;
}

/**
 * @brief 检测ipv4数据包的正确性
 *
 * @param ipv4_pkt
 * @param pkt_size
 * @param netif
 * @return net_err_t
 */
static net_err_t ipv4_pkt_check(ipv4_pkt_t *ipv4_pkt, uint16_t pkt_size,
                                const netif_t *netif) {
  // 检查ipv4版本号
  if (ipv4_pkt->hdr.version != IPV4_VERSION) {
    dbg_warning(DBG_IPV4, "ipv4 version error.");
    return NET_ERR_IPV4;
  }

  // 检查ipv4头部长度
  int hdr_size = ipv4_hdr_size(ipv4_pkt);
  if (hdr_size < sizeof(ipv4_hdr_t)) {
    dbg_warning(DBG_IPV4, "ipv4 header size error.");
    return NET_ERR_IPV4;
  }

  // 检查ipv4数据包总长度
  int total_size = net_ntohs(ipv4_pkt->hdr.total_len);
  if (total_size < hdr_size || total_size > pkt_size) {
    dbg_warning(DBG_IPV4, "ipv4 total size error.");
    return NET_ERR_IPV4;
  }

  // 检查头部校验和
  if (ipv4_pkt->hdr.hdr_chksum) {
    //* 进行校验和计算时，ip头部要保持原有字节序
    uint16_t chksum = tools_checksum16(&ipv4_pkt->hdr, hdr_size, 0, 1);
    if (chksum != 0) {
      dbg_warning(DBG_IPV4, "ipv4 header checksum error.");
      return NET_ERR_IPV4;
    }
  }

  return NET_ERR_OK;
}

/**
 * @brief 将ipv4数据包头部从网络字节序转换为主机字节序
 *
 * @param hdr
 */
static void ipv4_hdr_ntoh(ipv4_hdr_t *hdr) {
  hdr->total_len = net_ntohs(hdr->total_len);
  hdr->id = net_ntohs(hdr->id);
  hdr->flags_frag_offset = net_ntohs(hdr->flags_frag_offset);
}

/**
 * @brief 处理接收到的ipv4数据包，且不需要进行分片处理
 * 主要完成多路复用处理，根据数据包协议字段将数据包提供给上层协议处理
 * 如：传输层(TCP, UDP) 和 同属于网络层的ICMP协议
 *
 * @param netif 接受到数据包的网络接口
 * @param buf ipv4数据包
 */
static net_err_t ipv4_handle_normal(const netif_t *netif, pktbuf_t *buf) {
  // 获取ipv4数据包结构
  ipv4_pkt_t *pkt = (ipv4_pkt_t *)pktbuf_data_ptr(buf);

  // 根据数据包的协议字段将其提供给上一层协议处理
  switch (pkt->hdr.tran_proto) {
    case NET_PROTOCOL_ICMPv4: {
      dbg_info(DBG_IPV4, "recv ICMPv4 packet.");
    } break;

    case NET_PROTOCOL_UDP: {
      dbg_info(DBG_IPV4, "recv UDP packet.");
    } break;

    case NET_PROTOCOL_TCP: {
      dbg_info(DBG_IPV4, "recv TCP packet.");
    } break;

    default: {
      dbg_warning(DBG_IPV4, "unknown transport layer protocol!");
    } break;
  }

  return NET_ERR_IPV4;
}




/**
 * @brief ipv4协议层对接收到的数据包进行处理
 *
 * @param netif 接收到数据包的网络接口
 * @param buf 接收到的数据包
 * @return net_err_t
 */
net_err_t ipv4_recv(const netif_t *netif, pktbuf_t *buf) {
  dbg_info(DBG_IPV4, "recv ipv4 packet....");

  net_err_t err = NET_ERR_OK;

  // 设置数据包头部在内存上的连续性
  err = pktbuf_set_cont(buf, sizeof(ipv4_hdr_t));
  if (err != NET_ERR_OK) {
    dbg_error(DBG_IPV4, "recv ipv4 packet error: pktbuf set cont failed.");
    return err;
  }

  // 获取ipv4数据包
  ipv4_pkt_t *ipv4_pkt = pktbuf_data_ptr(buf);
  if (ipv4_pkt == (ipv4_pkt_t *)0) {
    dbg_error(DBG_IPV4, "recv ipv4 packet error: get ipv4 failed.");
    return NET_ERR_IPV4;
  }

  // 检查ipv4数据包
  err = ipv4_pkt_check(ipv4_pkt, pktbuf_total_size(buf), netif);
  if (err != NET_ERR_OK) {
    dbg_warning(DBG_IPV4, "check failed.");
    return err;
  }

  // 将ipv4数据包头部从网络字节序转换为主机字节序
  ipv4_hdr_ntoh(&ipv4_pkt->hdr);

  // 调整数据包大小，移除尾部的填充字节
  err = pktbuf_resize(buf, ipv4_pkt->hdr.total_len);
  if (err != NET_ERR_OK) {
    dbg_warning(DBG_IPV4, "resize failed.");
    return err;
  }

  // 判断是否为发送给本网络接口的数据包
  ipaddr_t dest_ip;
  ipaddr_from_bytes(&dest_ip, ipv4_pkt->hdr.dest_ip);
  if (!ipaddr_is_match(&dest_ip, &netif->ipaddr, &netif->netmask)) {  // 该数据包不是发给本接口的
    dbg_warning(DBG_IPV4, "ipv4 packet not for me.");
    // TODO: 后续可以添加路由表，进行数据包转发
    return NET_ERR_IPV4;
  }

  // 使用ipv4_normal_recv()函数处理数据包
  ipv4_handle_normal(netif, buf);  //!!! 数据包传递

  dbg_info(DBG_IPV4, "recv ipv4 packet ok.");
  return NET_ERR_OK;
}