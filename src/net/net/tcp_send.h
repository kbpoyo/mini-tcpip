/**
 * @file tcp_send.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief tcp传输协议模块发送数据的相关处理方法
 * @version 0.1
 * @date 2024-10-12
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef TCP_SEND_H
#define TCP_SEND_H

#include "tcp.h"



net_err_t tcp_transmit(tcp_t *tcp);
net_err_t tcp_send_reset(tcp_info_t *info);
net_err_t tcp_send_syn(tcp_t *tcp);
net_err_t tcp_send_ack(tcp_t *tcp, tcp_info_t *info);
net_err_t tcp_send_fin(tcp_t *tcp);
int tcp_send_bufwrite(tcp_t *tcp, const uint8_t *buf, int len);


#endif  // TCP_SEND_H