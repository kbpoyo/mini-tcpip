/**
 * @file icmpv4.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief icmpv4协议处理模块
 * @version 0.1
 * @date 2024-07-05
 *
 * 对icmpv4协议数据包进行接收和发送处理
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "icmpv4.h"

#include "dbg.h"
#include "ipv4.h"
#include "net_err.h"
#include "protocol.h"

#if DBG_DISP_ENABLED(DBG_ICMPV4)

/**
 * @brief 打印icmp数据包
 *
 * @param icmp_pkt
 */
static void icmpv4_pkt_display(const icmpv4_pkt_t *icmpv4_pkt) {
  plat_printf("---------------- icmpv4 packet ----------------\n");

  char *type = 0;
  switch (icmpv4_pkt->hdr.type) {
    case ICMPv4_TYPE_ECHO_REQUEST:
      type = "echo request";
      break;
    case ICMPv4_TYPE_ECHO_REPLY:
      type = "echo reply";
      break;
    default:
      type = "unknown";
      break;
  }
  plat_printf("\ttype: (%d) %s\n", icmpv4_pkt->hdr.type, type);
  plat_printf("\tcode: %d\n", icmpv4_pkt->hdr.code);
  plat_printf("\tchecksum: 0x%04x\n", icmpv4_pkt->hdr.chksum);

  plat_printf("---------------------------------------------\n");
}

#else

#define icmpv4_pkt_display(icmpv4_pkt)
#endif

/**
 * @brief 初始化icmpv4协议处理模块
 *
 * @return net_err_t
 */
net_err_t icmpv4_module_init(void) {
  dbg_info(DBG_ICMPV4, "init icmpv4 module....");

  dbg_info(DBG_ICMPV4, "init icmpv4 module ok.");
  return NET_ERR_OK;
}

/**
 * @brief 检查icmpv4数据包是否合法
 *
 * @param icmpv4_pktbuf
 * @return net_err_t
 */
static net_err_t icmpv4_pkt_check(pktbuf_t *icmpv4_pktbuf) {
  // 获取icmpv4数据包大小
  uint32_t pkt_size = pktbuf_total_size(icmpv4_pktbuf);

  // 检查icmpv4数据包长度是否合法
  if (pkt_size < sizeof(icmpv4_hdr_t)) {
    dbg_warning(DBG_ICMPV4, "icmpv4 pkt size error.");
    return NET_ERR_ICMPv4;
  }

  // 检查icmpv4包的校验和(包含整个icmpv4数据包)
  uint16_t chksum = pktbuf_checksum16(icmpv4_pktbuf, pkt_size, 0, 1);
  if (chksum != 0) {
    dbg_warning(DBG_ICMPV4, "icmpv4 pkt checksum error.");
    return NET_ERR_ICMPv4;
  }

  return NET_ERR_OK;
}

/**
 * @brief 发送一个icmpv4数据包
 *
 * @param dest_ipaddr 目的主机ip地址
 * @param src_ipaddr 发送主机ip地址
 * @param icmpv4_buf icmpv4数据包
 * @return net_err_t
 */
static net_err_t icmpv4_send(const ipaddr_t *dest_ipaddr,
                             const ipaddr_t *src_ipaddr, pktbuf_t *icmpv4_buf) {
  dbg_info(DBG_ICMPV4, "send icmpv4 packet....");

  // 获取icmpv4数据包
  icmpv4_pkt_t *icmpv4_pkt = (icmpv4_pkt_t *)pktbuf_data_ptr(icmpv4_buf);
  // 计算数据包校验和
  pktbuf_acc_reset(
      icmpv4_buf);  // 重置数据包访问位置，以使计算校验和时从icmpv4头部开始
  icmpv4_pkt->hdr.chksum =
      pktbuf_checksum16(icmpv4_buf, pktbuf_total_size(icmpv4_buf), 0, 1);

  // 打印icmpv4数据包
  icmpv4_pkt_display(icmpv4_pkt);

  // 将数据包交给ipv4层发送
  return ipv4_send(NET_PROTOCOL_ICMPv4, dest_ipaddr, src_ipaddr, icmpv4_buf);
}

/**
 * @brief 根据接收到的icmpv4回显请求数据包，发送一个icmpv4回显应答数据包
 *
 * @param icmpv4_pktbuf
 * @return net_err_t
 */
static net_err_t  icmpv4_make_echo_reply(const ipaddr_t *dest_ipaddr,
                                        const ipaddr_t *src_ipaddr,
                                        pktbuf_t *icmpv4_pktbuf) {
  // 获取icmpv4数据包
  icmpv4_pkt_t *icmpv4_pkt = (icmpv4_pkt_t *)pktbuf_data_ptr(icmpv4_pktbuf);

  // 修改icmpv4数据包类型为回显应答
  icmpv4_pkt->hdr.type = ICMPv4_TYPE_ECHO_REPLY;
  icmpv4_pkt->hdr.chksum = 0;  // 先清除校验和字段, 等计算完校验和再填入

  // 发送icmpv4数据包
  return icmpv4_send(dest_ipaddr, src_ipaddr, icmpv4_pktbuf);
}

/**
 * @brief 接收一个icmpv4数据包
 *7
 * @param dest_ipaddr 数据包目的ip地址
 * @param src_ipaddr 数据包源ip地址
 * @param buf ipv4数据包, 包含ipv4头部
 * @return net_err_t
 */
net_err_t icmpv4_recv(const ipaddr_t *dest_ipaddr, const ipaddr_t *src_ipaddr,
                      pktbuf_t *buf) {
  dbg_info(DBG_ICMPV4, "recv icmpv4 packet....");

  pktbuf_check_buf(buf);
  net_err_t err = NET_ERR_OK;

  // // 确保ipv4头部和icmpv4头部在内存上连续
  // ipv4_pkt_t *ipv4_pkt = (ipv4_pkt_t *)pktbuf_data_ptr(buf);
  // int ipv4_hdr_size = ipv4_get_hdr_size(ipv4_pkt);
  // err = pktbuf_set_cont(buf, ipv4_hdr_size + sizeof(icmpv4_hdr_t));
  // if (err != NET_ERR_OK) {
  //   dbg_error(DBG_ICMPV4, "pktbuf set cont failed.");
  //   return err;
  // }

  // 再次获取ipv4数据包并移除ipv4头部
  err = pktbuf_header_remove(
      buf, ipv4_get_hdr_size((ipv4_pkt_t *)pktbuf_data_ptr(buf)));
  if (err != NET_ERR_OK) {
    dbg_error(DBG_ICMPV4, "pktbuf header remove failed.");
    return err;
  }

  // 确保icmpv4头部在内存上连续
  err = pktbuf_set_cont(buf, sizeof(icmpv4_hdr_t));
  if (err != NET_ERR_OK) {
    dbg_error(DBG_ICMPV4, "pktbuf set cont failed.");
    return err;
  }

  // 检查icmpv4数据包是否合法
  err = icmpv4_pkt_check(buf);
  if (err != NET_ERR_OK) {
    dbg_warning(DBG_ICMPV4, "icmpv4 pkt check failed.");
    return err;
  }

  // 获取icmpv4数据包
  icmpv4_pkt_t *icmpv4_pkt = (icmpv4_pkt_t *)pktbuf_data_ptr(buf);
  // 打印icmpv4数据包
  icmpv4_pkt_display(icmpv4_pkt);

  // 根据icmpv4数据包类型进行处理
  switch (icmpv4_pkt->hdr.type) {
    case ICMPv4_TYPE_ECHO_REQUEST: {
      // 处理icmpv4请求报文, 直接发送icmpv4应答报文即可
      return icmpv4_make_echo_reply(src_ipaddr, dest_ipaddr,
                                    buf);  //!!! 数据包传递
    } break;

    case ICMPv4_TYPE_ECHO_REPLY: {
    } break;

    default: {
      // TODO: icmp控制报文类型较多，以后再添加其他类型的处理, 此处仅释放数据包
      dbg_warning(DBG_ICMPV4, "unknown icmpv4 pkt type.");
      pktbuf_free(buf);
    } break;
  }

  return NET_ERR_OK;
}

/**
 * @brief 发送一个icmpv4不可达报文
 * 数据格式：
 *  0      7 8   15 16                  32
 *  ———————————————————————————————————— ——————> ipv4_pkt
 *  |       20Byte IPv4 Header         |
 *  ———————————————————————————————————— ——————> icmp_pkt
 *  | type | code |     checksum       |
 *  ————————————————————————————————————
 *  |          unused (4Byte)          |
 *  ———————————————————————————————————— ——————> 原始ip数据报
 *  | 不超过548Byte的原始ipv4数据报内容  |
 *  ————————————————————————————————————
 * (新生成的ipv4数据报大小需要不大于576字节)
 *
 * @param dest_ipaddr 目的主机ip地址
 * @param src_ipaddr 本地主机ip地址
 * @param unreach_code 根据code值不同，表示不可达的类型
 * @param buf 导致发送不可达报文的ipv4数据包
 * @return net_err_t
 */
net_err_t icmpv4_make_unreach(const ipaddr_t *dest_ipaddr,
                              const ipaddr_t *src_ipaddr, uint8_t unreach_code,
                              pktbuf_t *ipv4_buf) {
  dbg_info(DBG_ICMPV4, "send an icmpv4 unreach packet....");
  net_err_t err = NET_ERR_OK;

  // 计算需要从原始数据包了拷贝的字节数
  // 待拷贝字节数 copy_size = 不可达报文最大大小 - ipv4头部大小 - icmpv4头部大小
  int copy_size =
      ICMPv4_UNREACH_PKT_MAX_SIZE - sizeof(ipv4_hdr_t) - sizeof(icmpv4_hdr_t);
  int buf_size = pktbuf_total_size(ipv4_buf);
  copy_size = copy_size > buf_size ? buf_size : copy_size;

  // 构造icmp报文
  pktbuf_t *icmp_buf =
      pktbuf_alloc(sizeof(icmpv4_hdr_t) + copy_size);  //!!! 分配数据包
  if (!icmp_buf) {
    dbg_error(DBG_ICMPV4, "alloc buf failed!");
    return NET_ERR_ICMPv4;
  }
  icmpv4_hdr_t *icmp_hdr = (icmpv4_hdr_t *)pktbuf_data_ptr(icmp_buf);
  icmp_hdr->type = ICMPv4_TYPE_UNREACH;
  icmp_hdr->code = unreach_code;
  icmp_hdr->chksum = 0;   // 校验和先置0，后续再计算
  icmp_hdr->reserve = 0;  // 未使用该字段，置0即可

  // 将原始的ip数据报的copy_size个字节拷贝到icmp不可达报文中作为负载
  pktbuf_acc_reset(ipv4_buf);  // 重置ipv4_buf的访问位置，从头开始拷贝
  pktbuf_seek(icmp_buf, sizeof(icmpv4_hdr_t));  // 将内容拷贝到icmp_buf的负载区
  err = pktbuf_copy(icmp_buf, ipv4_buf, copy_size);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_ICMPV4, "copy original datagram failed!");
    pktbuf_free(icmp_buf);  //!!! 数据包释放
    return err;
  }

  // 发送icmp不可达报文
  err = icmpv4_send(dest_ipaddr, src_ipaddr, icmp_buf);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_ICMPV4, "send icmp packet failed!");
    pktbuf_free(icmp_buf);  //!!! 释放数据包
    return err;
  }

  return NET_ERR_OK;
}