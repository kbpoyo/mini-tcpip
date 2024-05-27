/**
 * @file protocol.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 协议相关信息
 * @version 0.1
 * @date 2024-05-27
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H
/**
 * @brief 协议类型的枚举
 * 
 */
typedef enum _protocol_type_t {
  NET_PROTOCOL_NONE = 0,       // 无协议
  NET_PROTOCOL_ARP = 0x0806,   // ARP协议
  NET_PROTOCOL_IPV4 = 0x0800,    // IP协议
  NET_PROTOCOL_IPV6 = 0x86DD,  // IPV6协议
} protocol_type_t;

#endif  // PROTOCOL_H