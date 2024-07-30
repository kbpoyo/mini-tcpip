/**
 * @file socket.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief socket相关编程接口
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
#undef AF_INET
#define AF_INET 2

// 定义ip地址结构
struct net_in_addr {
    union {
        struct {
            uint8_t s_addr0;
            uint8_t s_addr1;
            uint8_t s_addr2;
            uint8_t s_addr3;
        };
        
        uint8_t s_addr_bytes[IPV4_ADDR_SIZE];
        #undef s_addr
        uint32_t s_addr;
    };
};


// 定义socket结构，描述端对端的连接信息
struct net_sockaddr_in {
    uint8_t sin_len;
    uint8_t sin_family; // 协议簇
	uint16_t sin_port; // 端口号
	struct net_in_addr	sin_addr; //网络地址
	char	sin_zero[8]; 
};



#endif  // SOCKET_H