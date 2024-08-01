/**
 * @file sock.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 协议栈内部socket相关数据结构及方法
 * @version 0.1
 * @date 2024-07-31
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef SOCK_H
#define SOCK_H

#include "exmsg.h"
#include "ipaddr.h"
#include "net_err.h"
#include "nlist.h"
#include "socket.h"

struct net_sockaddr;
struct _sock_t;
// 抽象基础socket的操作接口
typedef struct _sock_ops_t {
  net_err_t (*close)(struct _sock_t *sock);  // 关闭socket
  // 向目标socket地址发送信息
  net_err_t (*sendto)(struct _sock_t *sock, const void *buf, size_t buf_len,
                      int flags, const struct net_sockaddr *dest,
                      net_socklen_t dest_len,
                      ssize_t *ret_send_len);  // 发送数据

  // 从目标socket地址接收信息
  net_err_t (*recvfrom)(struct _sock_t *sock, void *buf, size_t buf_len,
                        int flags, const struct net_sockaddr *src,
                        net_socklen_t src_len, ssize_t *ret_recv_len);

  // 设置socket选项
  net_err_t (*setopt)(struct _sock_t *sock, int level, int optname,
                      const char *optval);

  // 销毁socket对象
  void (*destroy)(struct _sock_t *sock);

} sock_ops_t;

// 定义基础socket结构, 描述端与端应用程序通信的基本信息
typedef struct _sock_t {
  nlist_node_t node;  // 用于挂载到socket对象链表的节点

  uint16_t local_port;   // 本地端口
  ipaddr_t local_ip;     // 本地IP地址
  uint16_t remote_port;  // 远端端口
  ipaddr_t remote_ip;    // 远端IP地址

  int family;    // 网络层地址族(AF_INET, AF_INET6):(IPv4, IPv6)
  int protocol;  // 传输层协议(udp,tcp)或icmp协议

  int err_code;  // socket错误码
  int recv_tmo;  // 接收超时时间
  int send_tmo;  // 发送超时时间

  const sock_ops_t *ops;  // socket操作接口
} sock_t;

// 定义外部socket结构，对内部sock_t结构的封装
typedef struct _net_socket {
  nlist_node_t node;  // 用于挂载到socket对象链表的节点
  enum {
    SOCKET_STATE_FREE = 0,  // 空闲
    SOCKET_STATE_USED,
  } state;

  sock_t *sock;  // 内部socket对象

} net_socket_t;

// socket创建请求的参数结构
typedef struct _sock_create_t {
  int family;
  int type;
  int protocol;
} sock_create_t;

// socket发送和接收请求(sendto和recvfrom)的参数结构
typedef struct _sock_io_t {
  const void *buf;
  size_t buf_len;
  int flags;
  struct net_sockaddr *sockaddr;
  net_socklen_t sockaddr_len;
  ssize_t ret_len;
} sock_io_t, sock_sendto_t, sock_recvfrom_t;

// 用于封装外部线程请求工作线程执行socket相关操作时传递的参数
typedef struct _sock_req_t {
  int sock_fd;  // socket文件描述符
  union {
    sock_create_t create;  // 创建socket请求的参数
    sock_io_t io;
  };

} sock_req_t;

net_err_t sock_module_init(void);
net_err_t sock_init(sock_t *sock, int family, int protocol, sock_ops_t *ops);

net_err_t sock_req_creat(msg_func_t *msg);
net_err_t sock_req_sendto(msg_func_t *msg);

#endif  // SOCK_H