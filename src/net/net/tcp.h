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
#include "tools.h"

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
  ipaddr_t local_ip;   // 本地ip地址
  ipaddr_t remote_ip;  // 远端ip地址
  tcp_hdr_t *tcp_hdr;  // tcp头部
  pktbuf_t *tcp_buf;   // tcp数据包
  uint32_t data_len;   // 数据长度
  uint32_t seq;        // 序号
  uint32_t seq_len;    // 序号长度
} tcp_info_t;

// 定义tcp状态
typedef enum _tcp_state_t {
  TCP_STATE_CLOSED = 0,   // 关闭状态
  TCP_STATE_LISTEN,       // 监听状态
  TCP_STATE_SYN_SENT,     // SYN已发送
  TCP_STATE_SYN_RCVD,     // SYN已接收
  TCP_STATE_ESTABLISHED,  // 连接已建立
  TCP_STATE_FIN_WAIT_1,   // FIN已发送
  TCP_STATE_FIN_WAIT_2,   // FIN已接收
  TCP_STATE_CLOSING,      // 关闭中
  TCP_STATE_TIME_WAIT,    // 等待超时
  TCP_STATE_CLOSE_WAIT,   // 等待关闭
  TCP_STATE_LAST_ACK,     // 最后确认
  TCP_STATE_MAX,          // 辅助状态，用于判断状态是否合法

} tcp_state_t;

// 定义tcp socket结构, 派生自基础socket结构
typedef struct _tcp_t {
  sock_t sock_base;  // 基础socket结构(父类，必须在第一个位置)

  tcp_state_t state;  // tcp状态

  // tcp标志位
  struct {
    uint32_t syn_send : 1;  // SYN已发送
    uint32_t recv_win_valid : 1;   // 接收窗口的isn是否有效
  } flags;

  struct {             // 用于处理tcp连接的结构
    sock_wait_t wait;  // 用于处理tcp 连接的等待事件
  } conn;

  // 发送窗口
  // [isn ~ una) 已确认的数据, [una ~ nxt) 已发送未确认的数据, [nxt ~ end)
  struct {
    uint32_t isn;      // 初始序号
    uint32_t una;      // 未确认的数据的数据段序号
    uint32_t nxt;      // 下一个将要发送的数据段序号
    sock_wait_t wait;  // 用于处理tcp发送的等待事件
  } send;

  // 接收窗口
  // [isn ~ nxt) 已接收的数据, [nxt ~ end) 等待接收的数据
  struct {
    uint32_t isn;      // 初始序号
    uint32_t nxt;      // 下一个将要接收的数据段序号
    sock_wait_t wait;  // 用于处理tcp接收的等待事件
  } recv;

} tcp_t;

net_err_t tcp_module_init(void);
sock_t *tcp_create(int family, int protocol);
void tcp_info_init(tcp_info_t *tcp_info, pktbuf_t *tcp_buf, ipaddr_t *dest_ip,
                   ipaddr_t *src_ip);
tcp_t *tcp_find(tcp_info_t *info);
net_err_t tcp_abort_connect(tcp_t *tcp, net_err_t err);

static inline int tcp_get_hdr_size(const tcp_hdr_t *tcp_hdr) {
  return (tcp_hdr->hdr_len * 4);
}

static inline void tcp_set_hdr_size(tcp_hdr_t *tcp_hdr, int size) {
  tcp_hdr->hdr_len = size / 4;
}

/**
 * @brief 将头部字段中除开校验和以外的字段转换为网络字节序
 *
 * @param hdr
 */
static inline void tcp_hdr_hton(tcp_hdr_t *hdr) {
  hdr->src_port = net_htons(hdr->src_port);
  hdr->dest_port = net_htons(hdr->dest_port);
  hdr->seq = net_htonl(hdr->seq);
  hdr->ack = net_htonl(hdr->ack);
  hdr->win_size = net_htons(hdr->win_size);
  hdr->urg_ptr = net_htons(hdr->urg_ptr);
}

/**
 * @brief 将头部字段中除开校验和以外的字段转换为主机字节序
 *
 * @param hdr
 */
static inline void tcp_hdr_ntoh(tcp_hdr_t *hdr) {
  hdr->src_port = net_ntohs(hdr->src_port);
  hdr->dest_port = net_ntohs(hdr->dest_port);
  hdr->seq = net_ntohl(hdr->seq);
  hdr->ack = net_ntohl(hdr->ack);
  hdr->win_size = net_ntohs(hdr->win_size);
  hdr->urg_ptr = net_ntohs(hdr->urg_ptr);
}

// 定义类型检查宏
#define type_check(type, var)   \
  ({                            \
    type __var1;                \
    typeof(var) __var2;   \
    (void)(&__var1 == &__var2); \
    1;                          \
  })

// 定义序号比较宏(当b在逻辑上减a的值不超过2^31时，即使出现回绕该比较结果也正确) 1：a < b 0：a >= b
#define tcp_seq_before(a, b) \
(type_check(uint32_t, a) && type_check(uint32_t, b) && ((int32_t)(a) - (int32_t)(b) < 0))
#define tcp_seq_before_eq(a, b) \
(type_check(uint32_t, a) && type_check(uint32_t, b) && ((int32_t)(a) - (int32_t)(b) <= 0))
#define tcp_seq_after(a, b) tcp_seq_before(b, a)
#define tcp_seq_after_eq(a, b) tcp_seq_before_eq(b, a)
#define tcp_seq_between(a, b, c) \
(tcp_seq_after(a, b) && tcp_seq_before(c, b))
#define tcp_seq_between_eq(a, b, c) \
(tcp_seq_after_eq(a, b) && tcp_seq_before_eq(c, b))

#if DBG_DISP_ENABLED(DBG_TCP)
    void tcp_disp(char *msg, tcp_t *tcp);
void tcp_disp_pkt(char *msg, tcp_hdr_t *tcp_hdr, pktbuf_t *tcp_buf);
void tcp_disp_list(void);
#else

#define tcp_disp_sock(msg, tcp)
#define tcp_disp_pkt(msg, tcp_hdr, tcp_buf)
#define tcp_disp_list()

#endif

#endif  // TCP_H