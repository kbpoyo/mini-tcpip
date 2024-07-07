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
#include "icmpv4.h"
#include "protocol.h"
#include "tools.h"

#if DBG_DISP_ENABLED(DBG_IPV4)

/**
 * @brief 打印ipv4数据包
 *
 * @param ipv4_pkt
 */
static void ipv4_pkt_display(const ipv4_pkt_t *ipv4_pkt) {
  plat_printf("---------------- ipv4 packet ----------------\n");

  // 打印ipv4头部信息
  const ipv4_hdr_t *hdr = &ipv4_pkt->hdr;
  plat_printf("\tversion: %d\n", hdr->version);
  plat_printf("\thead len: %d\n", ipv4_get_hdr_size(ipv4_pkt));
  plat_printf("\ttotal len: %d\n", net_ntohs(hdr->total_len));
  plat_printf("\tid: %d\n", net_ntohs(hdr->id));
  plat_printf("\ttime to life: %d\n", hdr->ttl);
  char *tran_proto = 0;
  switch (hdr->tran_proto) {
    case NET_PROTOCOL_ICMPv4:
      tran_proto = "icmpv4";
      break;
    case NET_PROTOCOL_UDP:
      tran_proto = "udp";
      break;
    case NET_PROTOCOL_TCP:
      tran_proto = "tcp";
      break;
    default:
      tran_proto = "unknown";
      break;
  }
  plat_printf("\ttranspart protocol: (%d) %s\n", hdr->tran_proto, tran_proto);
  plat_printf("\theadr checksum: 0x%04x\n", hdr->hdr_chksum);
  ipaddr_t ipaddr;
  ipaddr_from_bytes(&ipaddr, hdr->src_ip);
  netif_dum_ip("\tsrc ip: ", &ipaddr);
  ipaddr_from_bytes(&ipaddr, hdr->dest_ip);
  netif_dum_ip("\tdest ip: ", &ipaddr);

  plat_printf("\n---------------------------------------------\n");
}

#else

#define ipv4_pkt_display(ipv4_pkt)
#endif

static inline int ipv4_get_id(void) {
  static int id = 0;
  return id++;
}

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
static net_err_t ipv4_pkt_check(ipv4_pkt_t *ipv4_pkt, uint16_t pkt_size) {
  // 检查ipv4版本号
  if (ipv4_pkt->hdr.version != IPV4_VERSION) {
    dbg_warning(DBG_IPV4, "ipv4 version error.");
    return NET_ERR_IPV4;
  }

  // 检查ipv4头部长度
  int hdr_size = ipv4_get_hdr_size(ipv4_pkt);
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
 * @brief 将ipv4数据包头部从主机字节序转换为网络字节序
 *
 * @param hdr
 */
static void ipv4_hdr_hton(ipv4_hdr_t *hdr) {
  hdr->total_len = net_htons(hdr->total_len);
  hdr->id = net_htons(hdr->id);
  hdr->flags_frag_offset = net_htons(hdr->flags_frag_offset);
}

/**
 * @brief 处理接收到的ipv4数据包，且不需要进行分片处理
 * 主要完成多路分解处理，根据数据包协议字段将数据包提供给上层协议处理
 * 如：传输层(TCP, UDP) 和 同属于网络层的ICMP协议
 *
 * @param netif 接受到数据包的网络接口
 * @param buf ipv4数据包
 */
static net_err_t ipv4_handle_normal(const netif_t *netif, pktbuf_t *buf) {
  net_err_t err = NET_ERR_OK;

  // 获取ipv4数据包结构
  ipv4_pkt_t *pkt = (ipv4_pkt_t *)pktbuf_data_ptr(buf);

  // 根据数据包的协议字段将其提供给上一层协议处理
  switch (pkt->hdr.tran_proto) {
    case NET_PROTOCOL_ICMPv4: {
      ipaddr_t src_ipaddr;
      ipaddr_from_bytes(&src_ipaddr, pkt->hdr.src_ip);
      // 当前网络接口已匹配该包的目的地址，直接使用接口地址即可
      err = icmpv4_recv(&netif->ipaddr, &src_ipaddr, buf);  //!!! 数据包传递
      if (err != NET_ERR_OK) {
        dbg_warning(DBG_IPV4, "icmpv4 recv failed.");
        return err;
      }
    } break;

    case NET_PROTOCOL_UDP: {  // TODO: 测试ICMP不可达报文
      dbg_info(DBG_IPV4, "recv UDP packet.");
      ipaddr_t src_ipaddr;
      ipv4_hdr_hton(&pkt->hdr);  // 将头部转换为网络字节序
      ipaddr_from_bytes(&src_ipaddr, pkt->hdr.src_ip);
      icmpv4_make_unreach(&src_ipaddr, &netif->ipaddr, ICMPv4_CODE_UNREACH_PORT,
                          buf);
      return NET_ERR_IPV4;
    } break;

    case NET_PROTOCOL_TCP: {
      dbg_info(DBG_IPV4, "recv TCP packet.");
    } break;

    default: {
      dbg_warning(DBG_IPV4, "unknown transport layer protocol!");
    } break;
  }

  return NET_ERR_OK;
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
  err = ipv4_pkt_check(ipv4_pkt, pktbuf_total_size(buf));
  if (err != NET_ERR_OK) {
    dbg_warning(DBG_IPV4, "check failed.");
    return err;
  }
  // 打印ipv4数据包头部信息
  ipv4_pkt_display(ipv4_pkt);

  // 将ipv4数据包头部从网络字节序转换为主机字节序
  ipv4_hdr_ntoh(&ipv4_pkt->hdr);

  // 判断是否为发送给本网络接口的数据包
  ipaddr_t dest_ip;
  ipaddr_from_bytes(&dest_ip, ipv4_pkt->hdr.dest_ip);
  if (!ipaddr_is_match(&dest_ip, &netif->ipaddr,
                       &netif->netmask)) {  // 该数据包不是发给本接口的
    dbg_warning(DBG_IPV4, "ipv4 packet not for me.");
    // TODO: 后续可以添加路由表，进行数据包转发
    return NET_ERR_IPV4;
  }

  // 调整数据包大小，移除尾部的填充字节
  err = pktbuf_resize(buf, ipv4_pkt->hdr.total_len);
  if (err != NET_ERR_OK) {
    dbg_warning(DBG_IPV4, "resize failed.");
    return err;
  }

  // 使用ipv4_normal_recv()函数处理数据包
  ipv4_handle_normal(netif, buf);  //!!! 数据包传递

  dbg_info(DBG_IPV4, "recv ipv4 packet ok.");
  return NET_ERR_OK;
}

/**
 * @brief 将数据包封装成ipv4数据包，并发送
 *
 * @param tran_protocol
 * @param dest_ipaddr
 * @param src_ipaddr
 * @param buf
 * @return net_err_t
 */
net_err_t ipv4_send(uint8_t tran_protocol, const ipaddr_t *dest_ipaddr,
                    const ipaddr_t *src_ipaddr, pktbuf_t *buf) {
  dbg_info(DBG_IPV4, "send an ipv4 packet....");

  net_err_t err = NET_ERR_OK;

  // 给数据包添加ipv4头部
  err = pktbuf_header_add(buf, sizeof(ipv4_hdr_t), PKTBUF_ADD_HEADER_CONT);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_IPV4, "add header failed.");
    return err;
  }

  // 获取ipv4头部对象
  ipv4_pkt_t *pkt = pktbuf_data_ptr(buf);
  // 设置ipv4头部信息
  pkt->hdr.version = IPV4_VERSION;
  ipv4_set_hdr_size(pkt, sizeof(ipv4_hdr_t));
  pkt->hdr.ecn = 0;
  pkt->hdr.diff_service = 0;
  pkt->hdr.total_len = pktbuf_total_size(buf);
  pkt->hdr.id = ipv4_get_id();
  pkt->hdr.flags_frag_offset = 0;
  pkt->hdr.ttl = NET_IPV4_DEFAULT_TTL;
  pkt->hdr.tran_proto = tran_protocol;
  pkt->hdr.hdr_chksum = 0;
  ipaddr_to_bytes(src_ipaddr, pkt->hdr.src_ip);
  ipaddr_to_bytes(dest_ipaddr, pkt->hdr.dest_ip);

  // 将头部转换为网络字节序
  ipv4_hdr_hton(&pkt->hdr);

  // 计算头部校验和
  pktbuf_acc_reset(buf);  // 重置buf访问位置，从头部开始计算校验和
  pkt->hdr.hdr_chksum = pktbuf_checksum16(buf, ipv4_get_hdr_size(pkt), 0, 1);

  // 打印ipv4数据包头部信息
  ipv4_pkt_display(pkt);

  // 通过网络接口发送数据包
  err = netif_send(netif_get_default(), dest_ipaddr, buf);  //!!! 数据包传递
  if (err != NET_ERR_OK) {
    dbg_error(DBG_IPV4, "netif send ip packet failed.");
  }

  return NET_ERR_OK;
}