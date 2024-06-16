/**
 * @file ipv4.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief ipv4协议模块
 * @version 0.1
 * @date 2024-06-15
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef IPV4_H
#define IPV4_H

#include <stdint.h>

#include "net_cfg.h"
#include "net_err.h"
#include "netif.h"

#define IPV4_ADDR_SIZE 4  // ipv4地址长度
#define IPV4_VERSION 4    // ipv4版本号

#pragma pack(1)

//!!! 小端模式下结构体位域的存储顺序是从低位到高位，而大端模式下是从高位到低位
/**
 * 小端:     |  ihl  |version|
 *          |7 6 5 4|3 2 1 0| (uint8_t)
 * 
 *          | ecn |diff_service|
 *          | 7 6 |5 4 3 2 1 0 | (uint8_t)
 *
 * 大端:     |version|  ihl  |
 *          |7 6 5 4|3 2 1 0| (uint8_t)
 * 
 *          |diff_service| ecn |
 *          |7 6 5 4 3 2 | 1 0 | (uint8_t)
 */

// 定义ipv4数据包头结构，设置固定长度20字节，不使用头部可选项
// 参考RFC0791
typedef struct _ipv4_hdr_t {
  union {
    struct {  // 4位版本号 + 4位头部长度
#if SYS_ENDIAN_LITTLE
      uint8_t ihl : 4;      // ipv4头部长度 = (ihl * 4) 字节
      uint8_t version : 4;  // ipv4版本号
#else
      uint8_t version : 4;
      uint8_t ihl : 4;
#endif
    };
    uint8_t domain_0_8;  // 版本和头部长度
  };

  union {
    struct {  // 6位区分服务 + 2位显式拥塞通知
#if SYS_ENDIAN_LITTLE
      uint8_t ecn : 2;  // 显式的拥塞通知explicit Congestion Notifications
      uint8_t diff_service : 6;  // 本意是用来区分服务，
                                 // 类似于vip可优先转发，但实际上基本不用
#else
      uint8_t diff_service : 6;
      uint8_t ecn : 2;
#endif
    };

    uint8_t domain_8_16;  // 区分服务和显式拥塞通知
  };

  uint16_t total_len;  // ip数据包总长度

  uint16_t id;                 // 标识
  uint16_t flags_frag_offset;  // 标志和片偏移

  uint8_t ttl;            // 生存跳数
  uint8_t tran_proto;     // 传输层协议类型
  uint16_t headr_chksum;  // 头部校验和

  uint8_t src_ip[IPV4_ADDR_SIZE];  // 源ip地址

  uint8_t dest_ip[IPV4_VERSION];  // 目的ip地址
} ipv4_hdr_t;

// 定义ipv4数据包结构
typedef struct _ipv4_pkt_t {
  ipv4_hdr_t hdr;   // ipv4头部
  uint8_t data[0];  // 数据负载
} ipv4_pkt_t;

#pragma pack()

net_err_t ipv4_module_init(void);

net_err_t ipv4_recv(const netif_t *netif, pktbuf_t *buf);

/**
 * @brief 获取数据包头部大小
 *
 * @param ipv4_pkt
 * @return int
 */
static inline int ipv4_hdr_size(ipv4_pkt_t *ipv4_pkt) {
  return (ipv4_pkt->hdr.ihl * 4);
}

#endif  // IPV4_H