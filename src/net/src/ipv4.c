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
  int total_size = ipv4_pkt->hdr.total_len;
  if (total_size < hdr_size || total_size > pkt_size) {
    dbg_warning(DBG_IPV4, "ipv4 total size error.");
    return NET_ERR_IPV4;
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

  // 将ipv4数据包头部从网络字节序转换为主机字节序
  ipv4_hdr_ntoh(&ipv4_pkt->hdr);

  // 检查ipv4数据包
  err = ipv4_pkt_check(ipv4_pkt, pktbuf_total_size(buf), netif);
  if (err != NET_ERR_OK) {
    dbg_warning(DBG_IPV4, "recv ipv4 packet warning: check failed.");
    return err;
  }

  // 调整数据包大小，移除尾部的填充字节
  err = pktbuf_resize(buf, ipv4_pkt->hdr.total_len);
  if (err != NET_ERR_OK) {
    dbg_warning(DBG_IPV4, "recv ipv4 packet warning: resize failed.");
    return err;
  }

 

  //!!! 释放数据包
  pktbuf_free(buf);

  dbg_info(DBG_IPV4, "recv ipv4 packet ok.");
  return NET_ERR_OK;
}