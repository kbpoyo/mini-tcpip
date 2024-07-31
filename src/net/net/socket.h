/**
 * @file socket.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 提供给应用程序的socket相关编程接口
 * @version 0.1
 * @date 2024-07-30
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef SOCKET_H
#define SOCKET_H

#include <stdint.h>

#include "ipv4.h"

// 定义网络地址默认值
#undef INADDR_ANY
#define INADDR_ANY 0x00000000

// 定义协议族
#undef AF_INET  // ipv4
#define AF_INET 2

// 定义socket类型
#undef SOCK_RAW  // 原始套接字
#define SOCK_RAW 0

// 定义协议类型
#undef IPPROTO_IP  // ip协议
#define IPPROTO_IP 0

// 定义网络(ip)地址结构
struct net_in_addr {
  union {
    struct {
      uint8_t s_addr0;
      uint8_t s_addr1;
      uint8_t s_addr2;
      uint8_t s_addr3;
    };

    uint8_t s_addr_bytes[IPV4_ADDR_SIZE];
#undef s_addr  // 为了避免本地协议栈重定义，先取消宏定义
    uint32_t s_addr;
  };
};

// 通用的, 兼容linux系统socket的地址结构
struct net_sockaddr {  // 结构内存大小与net_sockaddr_in相同
  uint8_t sin_len;
  uint8_t sin_family;
  uint8_t sin_data[14];
};

// 定义socket地址结构，描述端对端的基础连接信息
struct net_sockaddr_in {  // 结构内存大小与net_sockaddr相同
  uint8_t sin_len;
  uint8_t sin_family;           // 网络层协议簇
  uint16_t sin_port;            // 应用层端口号
  struct net_in_addr sin_addr;  // 网络层地址
  char sin_zero[8];
};

int net_socket(int family, int type, int protocol);

#endif  // SOCKET_H