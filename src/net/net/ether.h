/**
 * @file ether.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 链路层：以太网协议层接口
 * @version 0.1
 * @date 2024-05-26
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef ETHER_H
#define ETHER_H

#include <stdint.h>

#include "net_err.h"
#include "netif.h"
#include "protocol.h"

#define ETHER_MAC_LEN 6  // mac地址长度
#define ETHER_MTU 1500   // 以太网最大传输单元
#define ETHER_DATA_MIN  46  // 以太网最小数据长度

#pragma pack(1)

/**
 * @brief 以太网帧头部结构
 */
typedef struct _ether_hdr_t {
  uint8_t dest[ETHER_MAC_LEN];  // 目的mac地址
  uint8_t src[ETHER_MAC_LEN];   // 源mac地址
  uint16_t protocol_type;       // 帧协议类型
} ether_hdr_t;

/**
 * @brief 以太网帧结构
 */
typedef struct _ether_pkt_t {
  ether_hdr_t hdr;          // 帧头部
  uint8_t data[ETHER_MTU];  // 数据负载
} ether_pkt_t;

#pragma pack()

net_err_t ether_module_init(void);

const uint8_t *ether_broadcast_addr(void);

net_err_t ether_raw_send(netif_t *netif, protocol_type_t protocol,  const uint8_t *dest_mac_addr, pktbuf_t *buf);

#endif  // ETHER_H