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
#undef SOCK_DGRAM  // 数据报套接字
#define SOCK_DGRAM 1

// 定义协议类型(传输层协议 或 icmp协议)
#undef IPPROTO_ICMP  // icmp协议
#define IPPROTO_ICMP 1
#undef IPPROTO_UDP  // udp协议
#define IPPROTO_UDP 17

// 定义socket选项设置相关宏
#undef SOL_SOCKET  // 选项设置与哪一层(level)相关：通用socket选项(socket层)
#define SOL_SOCKET 0
#undef SO_RCVTIMEO  // 选项设置类型： 接收超时
#define SO_RCVTIMEO 1
#undef SO_SNDTIMEO  // 选项设置类型：发送超时
#define SO_SNDTIMEO 2

// 定义socket地址长度类型
typedef int net_socklen_t;

// 定义接口使用的时间类型，兼容linux系统
struct net_timeval {
  long tv_sec;   // 秒
  long tv_usec;  // 微秒
};

// 定义网络层(ip)地址结构
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
// 描述端对端的基础连接信息
struct net_sockaddr {  // 结构内存大小与net_sockaddr_in相同(兼容)
  uint8_t sa_len;
  uint8_t sa_family;
  uint8_t sa_data[14];
};

// 只适用于IPV4(AF_INET)的socket地址结构，
struct net_sockaddr_in {  // 结构内存大小与net_sockaddr相同
  uint8_t sin_len;
  uint8_t sin_family;           // 网络层协议簇
  uint16_t sin_port;            // 应用层端口号
  struct net_in_addr sin_addr;  // 网络层地址
  char sin_zero[8];
};

int net_socket(int family, int type, int protocol);
ssize_t net_sendto(int socket, const void *buf, size_t buf_len, int flags,
                   const struct net_sockaddr *dest, net_socklen_t dest_len);
ssize_t net_recvfrom(int socket, void *buf, size_t buf_len, int flags,
                     struct net_sockaddr *src, net_socklen_t *src_len);
int net_close(int socket);
int net_setsockopt(int socket, int level, int optname, const char *optval,
                   int optlen);

#endif  // SOCKET_H