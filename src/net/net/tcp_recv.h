/**
 * @file tcp_recv.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief tcp协议模块接收数据的相关处理方法
 * @version 0.1
 * @date 2024-10-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef TCP_RECV_H
#define TCP_RECV_H


#include "tcp.h"

net_err_t tcp_recv(pktbuf_t *tcp_buf, ipaddr_t *src_ip, ipaddr_t *dest_ip);


#endif  // TCP_RECV_H