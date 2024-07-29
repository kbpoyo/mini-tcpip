/**
 * @file ping.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 适配协议栈的ping程序
 * @version 0.1
 * @date 2024-07-29
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef PING_H
#define PING_H

#include <stdint.h>
#include <WinSock2.h>

#include "sys_plat.h"

#define IP_ADDR_SIZE 4
#define PING_BUF_SIZE 4096

#pragma pack(1)

typedef struct _ip_hdr_t { // ipv4数据包头部
  union {
    struct {                // 4位版本号 + 4位头部长度
      uint8_t ihl : 4;      // ipv4头部长度 = (ihl * 4) 字节
      uint8_t version : 4;  // ipv4版本号
    };
    uint8_t domain_0_8;  // 版本和头部长度
  };

  union {
    struct {            // 6位区分服务 + 2位显式拥塞通知
      uint8_t ecn : 2;  // 显式的拥塞通知explicit Congestion Notifications
      uint8_t diff_service : 6;  // 本意是用来区分服务，
                                 // 类似于vip可优先转发，但实际上基本不用
    };
    uint8_t domain_8_16;  // 区分服务和显式拥塞通知
  };

  uint16_t total_len;  // ip数据包总长度

  uint16_t id;  // 标识

  uint16_t flags_frag_offset;  // 标志和片偏移

  uint8_t ttl;          // 生存跳数
  uint8_t tran_proto;   // 传输层协议类型
  uint16_t hdr_chksum;  // 头部校验和

  uint8_t src_ip[IP_ADDR_SIZE];  // 源ip地址

  uint8_t dest_ip[IP_ADDR_SIZE];  // 目的ip地址
} ip_hdr_t;

typedef struct _icmp_hdr_t { // icmpv4头部
  uint8_t type;
  uint8_t code;
  uint16_t chksum;

  uint16_t id;
  uint16_t seq;

} icmp_hdr_t;


// ping请求包结构
typedef struct _echo_req_t {
    icmp_hdr_t icmp_hdr;
    char buf[PING_BUF_SIZE];
}echo_req_t;

// ping应答包结构
typedef struct _echo_reply_t {
    ip_hdr_t ip_hdr;
    icmp_hdr_t icmp_hdr;
    char buf[PING_BUF_SIZE];
} echo_reply_t;

#pragma pack()

// ping结构
typedef struct _ping_t {
    echo_req_t req;
    echo_reply_t reply;
} ping_t;

void ping_run(ping_t *ping, const char *dest_ip, int data_size, int count,
              int interval);

#endif  // PING_H