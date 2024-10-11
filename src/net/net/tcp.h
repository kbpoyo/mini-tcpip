/**
 * @file tcp.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief tcp传输协议模块
 * @version 0.1
 * @date 2024-10-10
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef TCP_H
#define TCP_H

#include "net_cfg.h"
#include "sock.h"

#pragma pack(1)
// 定义tcp 头部结构
typedef struct _tcp_hdr_t {
  uint16_t src_port;   // 源端口
  uint16_t dest_port;  // 目的端口
  uint32_t seq;        // 序号
  uint32_t ack;        // 确认号

#if SYS_ENDIAN_LITTLE
  // 小端模式下解析位字段
  struct {                 // 第13字节
    uint8_t reserved : 4;  // 保留位
    uint8_t hdr_len : 4;   // 头部长度(以4字节为一个单位)
  };

  union {
    struct {  // 第14字节，标志位
      uint8_t f_fin : 1;
      uint8_t f_syn : 1;
      uint8_t f_rst : 1;
      uint8_t f_psh : 1;
      uint8_t f_ack : 1;
      uint8_t f_urg : 1;
      uint8_t f_ece : 1;
      uint8_t f_cwr : 1;
    };
    uint8_t flag;
  };

#else
  // 大端模式下解析位字段
  struct {
    uint8_t hdr_len : 4;   // 头部长度(以4字节为一个单位)
    uint8_t reserved : 4;  // 保留位
  };

  union {
    struct {
      uint8_t f_cwr : 1;
      uint8_t f_ece : 1;
      uint8_t f_urg : 1;
      uint8_t f_ack : 1;
      uint8_t f_psh : 1;
      uint8_t f_rst : 1;
      uint8_t f_syn : 1;
      uint8_t f_fin : 1;
    };
    uint8_t flag;
  };

#endif

  uint16_t win_size;  // 滑动窗口大小
  uint16_t checksum;  // 校验和
  uint16_t urg_ptr;   // 紧急指针

} tcp_hdr_t;

typedef struct _tcp_pkt_t {
  tcp_hdr_t hdr;    // tcp头部
  uint8_t data[1];  // 数据负载
} tcp_pkt_t;

#pragma pack()


// 定义tcp包信息结构(描述一个tcp包的信息)
typedef struct _tcp_info_t {
    ipaddr_t local_ip;  // 本地ip地址
    ipaddr_t remote_ip; // 远端ip地址
    tcp_hdr_t *tcp_hdr; // tcp头部
    pktbuf_t *tcp_buf;  // tcp数据包
    uint32_t data_len;  // 数据长度
    uint32_t seq;       // 序号
    uint32_t seq_len;   // 序号长度
} tcp_info_t;

// 定义tcp socket结构, 派生自基础socket结构
typedef struct _tcp_t {
  sock_t sock_base;  // 基础socket结构
} tcp_t;

net_err_t tcp_module_init(void);
sock_t *tcp_create(int family, int protocol);

static inline int tcp_hdr_size(const tcp_hdr_t *tcp_hdr) {
  return (tcp_hdr->hdr_len * 4);
}

#endif  // TCP_H