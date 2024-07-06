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

#pragma pack(1)
// icmpv4头部
typedef struct _icmpv4_hdr_t {
  uint8_t type;
  uint8_t code;
  uint16_t chksum;
} icmpv4_hdr_t;

// icmpv4数据包
typedef struct _icmpv4_pkt_t {
  icmpv4_hdr_t hdr;

  union {   // icmpv4的16位的标识 + 16位的序列号
    uint32_t reserve;
  };

  uint8_t data[0];
} icmpv4_pkt_t;

#pragma pack()

// icmpv4报文类型枚举
typedef enum _icmpv4_type_t {
  ICMPv4_TYPE_ECHO_REQUEST = 8,
  ICMPv4_TYPE_ECHO_REPLY = 0,

} icmpv4_type_t;

// icmpv4报文代码枚举
typedef enum _icmpv4_code_t {
  ICMPv4_CODE_ECHO = 0,
} icmpv4_code_t;

net_err_t icmpv4_module_init(void);
net_err_t icmpv4_recv(const ipaddr_t *dest_ipaddr, const ipaddr_t *src_ipaddr,
                      pktbuf_t *buf);

#endif  // ICMPV4_H