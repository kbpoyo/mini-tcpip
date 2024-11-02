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

#define NET_PORT_EMPTY 0     // 空端口号
#define NET_PORT_START 1024  // 系统可用端口号起始值(1~1023为系统保留端口号)
#define NET_PORT_END 65535   // 系统可用端口号结束值

/**
 * @brief 协议类型的枚举
 *
 */
typedef enum _protocol_type_t {
  NET_PROTOCOL_NONE = 0,  // 无协议
  // 用于网络层和链路层(以太网)的多路复用协议
  NET_PROTOCOL_ARP = 0x0806,   // ARP协议
  NET_PROTOCOL_IPV4 = 0x0800,  // IP协议
  NET_PROTOCOL_IPV6 = 0x86DD,  // IPV6协议

  // 用于传输层和网络层(ip)的多路复用协议
  NET_PROTOCOL_ICMPv4 = 0x01,  // ICMPv4协议
  NET_PROTOCOL_UDP = 0x11,     // UDP协议
  NET_PROTOCOL_TCP = 0x06,     // TCP协议

} protocol_type_t;

#endif  // PROTOCOL_H