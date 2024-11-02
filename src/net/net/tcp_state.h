/**
 * @file tcp_state.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief tcp传输协议状态机处理模块
 * @version 0.1
 * @date 2024-10-24
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef TCP_STATE_H
#define TCP_STATE_H

#include "tcp.h"

// 定义tcp状态处理函数
typedef net_err_t (*tcp_state_handler_t)(tcp_t *tcp, tcp_info_t *info);

// 声明tcp接收状态处理函数
net_err_t tcp_state_handler_recv(tcp_t *tcp, tcp_info_t *info);

const char *tcp_state_name(tcp_state_t state);
void tcp_state_set(tcp_t *tcp, tcp_state_t state);
net_err_t tcp_ack_process(tcp_t *tcp, tcp_info_t *info);

#endif  // TCP_STATE_H