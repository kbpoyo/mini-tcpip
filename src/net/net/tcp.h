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
#include "tcp_buf.h"
#include "tools.h"

// 定义TCP默认最大报文长度(不包括ip和tcp头部)
// 主要避免在未知链路上传输时，因为MTU限制导致的分片
// 尽可能让tcp报文在链路上传输时不被分片
#define TCP_MSS_DEFAULT 536

// 定义tcp选项枚举
typedef enum _tcp_opt_t {  // TODO: 本协议栈只支持了部分选项
  TCP_OPT_END = 0,         // 结束选项
  TCP_OPT_NOP = 1,         // 无操作
  TCP_OPT_MSS = 2,         // 最大报文长度
  TCP_OPT_WSCALE = 3,      // 窗口扩大因子
  TCP_OPT_SACK_PERM = 4,   // SACK允许
  TCP_OPT_SACK = 5,        // SACK
  TCP_OPT_TS = 8,          // 时间戳
} tcp_opt_t;

#pragma pack(1)

// 定义tcp选项结构
typedef struct _tcp_opt_mss_t {  // 获取对端mss
  uint8_t kind;
  uint8_t len;
  uint16_t mss;
} tcp_opt_mss_t;

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
void tcp_info_init(tcp_info_t *tcp_info, pktbuf_t *tcp_buf, ipaddr_t *dest_ip,
                   ipaddr_t *src_ip);

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
  uint16_t
      mss;  // 对端一次性可接收的最大报文长度(不包括ip和tcp头部) = MTU - IP头部
            // - TCP头部
            // 设置mss主要是为了避免在未知链路上传输时，因为MTU限制导致的分片
            // 尽可能让tcp报文在链路上传输时不被分片

  // tcp标志位
  struct {
    uint32_t syn_need_send : 1;   // SYN需要发送
    uint32_t syn_need_ack : 1;    // SYN已发送, 需要ack确认
    uint32_t fin_need_send : 1;   // FIN需要发送
    uint32_t fin_need_ack : 1;    // FIN已发送, 需要ack确认
    uint32_t fin_recved : 1;     // 已接收对端的FIN
    uint32_t syn_recved : 1;     // 已接收对端的SYN
    uint32_t recv_win_valid : 1;  // 接收窗口的是否有效
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
    tcp_buf_t buf;     // tcp发送缓冲区
    // TODO:为了简化实现以及可移植到某些嵌入式设备上，这里直接使用一个固定大小的缓冲区，以后可以使用动态分配的缓冲区
    uint8_t buf_data[TCP_SBUF_SIZE];  // tcp发送缓冲区数据的数据区
  } send;

  // 接收窗口
  // [isn ~ nxt) 已接收的数据, [nxt ~ end) 等待接收的数据
  struct {
    uint32_t isn;                     // 初始序号
    uint32_t unr;                     // 未读取的数据的数据段序号
    uint32_t nxt;                     // 下一个将要接收的数据段序号
    sock_wait_t wait;                 // 用于处理tcp接收的等待事件
    tcp_buf_t buf;                    // tcp接收缓冲区
    uint8_t buf_data[TCP_RBUF_SIZE];  // tcp接收缓冲区数据的数据区
  } recv;

} tcp_t;
net_err_t tcp_module_init(void);
sock_t *tcp_create(int family, int protocol);
tcp_t *tcp_find(tcp_info_t *info);
net_err_t tcp_abort_connect(tcp_t *tcp, net_err_t err);
void tcp_read_options(tcp_t *tcp, tcp_hdr_t *tcp_hdr);
net_err_t tcp_write_options(tcp_t *tcp, pktbuf_t *buf);
int tcp_recv_window(tcp_t *tcp);
int tcp_seq_is_ok(tcp_t *tcp, tcp_info_t *info);

static inline int tcp_get_hdr_size(const tcp_hdr_t *tcp_hdr) {
  return (tcp_hdr->hdr_len * 4);
}

static inline void tcp_set_hdr_size(tcp_hdr_t *tcp_hdr, int size) {
  tcp_hdr->hdr_len = size / 4;
}

/**
 * @brief 获取tcp发送窗口中未确认的数据量, 不包括syn和fin请求
 *
 * @param tcp
 * @return int
 */
static inline int tcp_wait_ack_data(tcp_t *tcp) {
  return (tcp->send.nxt - tcp->send.una) -
         (tcp->flags.syn_need_ack + tcp->flags.fin_need_ack);
}

/**
 * @brief 获取tcp发送缓冲区中待发送的数据量
 *
 * @param tcp
 * @return int
 */
static inline int tcp_wait_send_data(tcp_t *tcp) {
  return tcp_buf_cnt(&tcp->send.buf) - tcp_wait_ack_data(tcp);
}

/**
 * @brief 获取tcp接收缓冲区中待接收的数据量
 * 
 * @param tcp 
 * @return int 
 */
static inline int tcp_wait_recv_data(tcp_t *tcp) {
  return (tcp->recv.nxt - tcp->recv.unr) - tcp->flags.fin_recved;
}

/**
 * @brief 获取tcp接收缓冲区中空闲的空间大小
 * 
 * @param tcp 
 * @return int 
 */
static inline int tcp_wait_free_cnt(tcp_t *tcp) {
  return tcp_buf_free_cnt(&tcp->recv.buf) - tcp_wait_recv_data(tcp);
}

/**
 * @brief 判断当前tcp是否已发送且已确认fin请求
 *
 * @param tcp
 * @return int
 */
#define tcp_fin_is_ack(tcp) \
  (tcp->flags.fin_need_send == tcp->flags.fin_need_ack)

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
    typeof(var) __var2;         \
    (void)(&__var1 == &__var2); \
    1;                          \
  })

// 定义序号比较宏(当b在逻辑上减a的值不超过2^31时，即使出现回绕该比较结果也正确)
// 1：a < b 0：a >= b
#define tcp_seq_before(a, b)                             \
  (type_check(uint32_t, a) && type_check(uint32_t, b) && \
   ((int32_t)(a) - (int32_t)(b) < 0))
#define tcp_seq_before_eq(a, b)                          \
  (type_check(uint32_t, a) && type_check(uint32_t, b) && \
   ((int32_t)(a) - (int32_t)(b) <= 0))
#define tcp_seq_after(a, b) tcp_seq_before(b, a)
#define tcp_seq_after_eq(a, b) tcp_seq_before_eq(b, a)
#define tcp_seq_between(a, b, c) (tcp_seq_after(c, a) && tcp_seq_before(b, a))
#define tcp_seq_between_eq(a, b, c) \
  (tcp_seq_after_eq(c, a) && tcp_seq_before_eq(b, a))

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