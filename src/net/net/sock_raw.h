/**
 * @file socket_raw.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 协议栈内部的原始socket相关数据结构及方法，
 * 派生自基础socket结构
 * @version 0.1
 * @date 2024-08-01
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef SOCKET_RAW_H
#define SOCKET_RAW_H

#include "sock.h"

// 定义原始socket结构, 派生自基础socket结构
typedef struct _sockraw_t {
  sock_t sock_base;  // 基础socket结构

  nlist_t recv_buf_list;  // 接收的数据包缓存链表
  sock_wait_t recv_wait;  // 用于处理raw socket的接收等待事件

} sockraw_t;

net_err_t sockraw_module_init(void);

sock_t *sockraw_create(int family, int protocol);
net_err_t sockraw_recv_pktbuf(pktbuf_t *raw_ip_buf);

#endif