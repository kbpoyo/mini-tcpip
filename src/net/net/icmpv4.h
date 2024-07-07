/**
 * @file icmpv4.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief icmpv4协议处理模块
 * @version 0.1
 * @date 2024-07-05
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef ICMPV4_H
#define ICMPV4_H

#include "ipaddr.h"
#include "pktbuf.h"

// icmpv4不可达报文的最大ipv4数据包大小
#define ICMPv4_UNREACH_PKT_MAX_SIZE 576

#pragma pack(1)
// icmpv4头部
typedef struct _icmpv4_hdr_t {
  uint8_t type;
  uint8_t code;
  uint16_t chksum;

  union {  // 可变的4字节数据部分，严格来说这4字节不算在头部中，但几乎每一个icmp报文都携带，即使未使用也填0

    // 由icmpv4报文类型决定
    // 回显请求与应答: icmpv4的16位的标识 + 16位的序列号
    // 其他类型的icmpv4报文对该4字节的使用不同
    // TODO: 本地未对其进行读写，暂时忽略其字节序
    uint32_t reserve;
  };
} icmpv4_hdr_t;

// icmpv4数据包
typedef struct _icmpv4_pkt_t {
  icmpv4_hdr_t hdr;

  uint8_t data[0];
} icmpv4_pkt_t;

#pragma pack()

// icmpv4报文类型枚举
typedef enum _icmpv4_type_t {
  ICMPv4_TYPE_ECHO_REQUEST = 8,
  ICMPv4_TYPE_ECHO_REPLY = 0,
  ICMPv4_TYPE_UNREACH = 3,

} icmpv4_type_t;

// icmpv4报文代码枚举
typedef enum _icmpv4_code_t {
  ICMPv4_CODE_ECHO = 0,
  ICMPv4_CODE_UNREACH_PORT = 3,
} icmpv4_code_t;

net_err_t icmpv4_module_init(void);
net_err_t icmpv4_recv(const ipaddr_t *dest_ipaddr, const ipaddr_t *src_ipaddr,
                      pktbuf_t *buf);

net_err_t icmpv4_make_unreach(const ipaddr_t *dest_ipaddr,
                              const ipaddr_t *src_ipaddr, uint8_t unreach_code,
                              pktbuf_t *ipv4_buf);

#endif  // ICMPV4_H