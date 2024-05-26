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

#define ETHER_MAC_LEN 6  // mac地址长度
#define ETHER_MTU 1500   // 以太网最大传输单元

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
typedef struct _ether_frame_t {
  ether_hdr_t hdr;          // 帧头部
  uint8_t data[ETHER_MTU];  // 数据负载
} ether_frame_t;

#pragma pack()

net_err_t ether_module_init(void);

#endif  // ETHER_H