/**
 * @file udp.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief udp传输协议模块
 * @version 0.1
 * @date 2024-09-26
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef UDP_H
#define UDP_H

#include "sock.h"

#define UDP_PORT_START 1024  // UDP可用端口号起始值
#define UDP_PORT_END 65535   // UDP可用端口号结束值

#pragma pack(1)
// 定义udp数据包头部结构
typedef struct _udp_hdr_t {
  uint16_t src_port;
  uint16_t dest_port;
  uint16_t total_len;
  uint16_t checksum;
}udp_hdr_t;


// 定义udp数据包结构
typedef struct _udp_pkt_t {
  udp_hdr_t udp_hdr;
  uint8_t data[1];

}udp_pkt_t;

#pragma pack()

// 定义udp socket结构, 派生自基础socket结构
typedef struct _udp_t {
  sock_t sock_base;  // 基础socket结构

  nlist_t recv_buf_list;  // 接收的数据包缓存链表
  sock_wait_t recv_wait;  // 用于处理udp socket的接收等待事件
} udp_t;

net_err_t udp_module_init(void);
sock_t *udp_create(int family, int protocol);
#endif
