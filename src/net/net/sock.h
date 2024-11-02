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

struct _sock_req_t;
struct net_sockaddr;
struct _sock_t;

// socket等待事件
#define SOCK_WAIT_NONE 0          // 无等待事件
#define SOCK_WAIT_READ (1 << 0)   // 等待读事件
#define SOCK_WAIT_WRITE (1 << 1)  // 等待写事件
#define SOCK_WAIT_CONN (1 << 2)   // 等待连接事件
#define SOCK_WAIT_ALL (SOCK_WAIT_READ | SOCK_WAIT_WRITE | SOCK_WAIT_CONN)
// socket等待事件结构
typedef struct _sock_wait_t {
  sys_sem_t sem;       // 信号量
  net_err_t error;     // 错误码
  int wait_event_cnt;  // 等待事件数
} sock_wait_t;

// socket_wait_t相关方法

net_err_t sock_wait_init(sock_wait_t *wait);
void sock_wait_destroy(sock_wait_t *wait);
net_err_t sock_wait_add(sock_wait_t *wait, int tmo, struct _sock_req_t *req);
net_err_t sock_wait_enter(sock_wait_t *wait, int tmo);
void sock_wait_leave(sock_wait_t *wait, net_err_t error);
void sock_wakeup(struct _sock_t *sock, int wait_type, int err);

// 抽象基础socket的操作接口
typedef struct _sock_ops_t {
  net_err_t (*close)(struct _sock_t *sock);  // 关闭socket
  // 向目标socket地址发送信息
  net_err_t (*sendto)(struct _sock_t *sock, const void *buf, size_t buf_len,
                      int flags, const struct net_sockaddr *dest,
                      net_socklen_t dest_len, ssize_t *ret_send_len);

  // 从目标socket地址接收信息
  net_err_t (*recvfrom)(struct _sock_t *sock, void *buf, size_t buf_len,
                        int flags, struct net_sockaddr *src,
                        net_socklen_t *src_len, ssize_t *ret_recv_len);

  // 设置socket选项
  net_err_t (*setopt)(struct _sock_t *sock, int level, int optname,
                      const char *optval, int optlen);

  // 连接目标socket地址
  net_err_t (*connect)(struct _sock_t *sock, const struct net_sockaddr *addr,
                       net_socklen_t addrlen);

  // 发送信息, 且socket已连接到远端地址
  net_err_t (*send)(struct _sock_t *sock, const void *buf, size_t buf_len,
                    int flags, ssize_t *ret_send_len);

  // 接收信息, 且socket已连接到远端地址
  net_err_t (*recv)(struct _sock_t *sock, void *buf, size_t buf_len, int flags,
                    ssize_t *ret_recv_len);

  // 绑定socket到本地地址
  net_err_t (*bind)(struct _sock_t *sock, const struct net_sockaddr *addr,
                    net_socklen_t addrlen);

  // 销毁socket对象
  void (*destroy)(struct _sock_t *sock);

} sock_ops_t;

// 基类已实现了的通用接口，派生类可重写或直接继承

net_err_t sock_setopt(struct _sock_t *sock, int level, int optname,
                      const char *optval, int optlen);
net_err_t sock_send(struct _sock_t *sock, const void *buf, size_t buf_len,
                    int flags, ssize_t *ret_send_len);
net_err_t sock_recv(struct _sock_t *sock, void *buf, size_t buf_len, int flags,
                    ssize_t *ret_recv_len);

// 定义基础socket结构, 描述端与端应用程序通信的基本信息
typedef struct _sock_t {
  nlist_node_t node;  // 用于挂载到socket对象链表的节点

  uint16_t local_port;   // 本地端口
  ipaddr_t local_ip;     // 本地IP地址
  uint16_t remote_port;  // 远端端口
  ipaddr_t remote_ip;    // 远端IP地址

  int family;    // 网络层地址族(AF_INET, AF_INET6):(IPv4, IPv6)
  int protocol;  // 传输层协议(udp,tcp)或icmp协议

  int err_code;            // socket错误码
  int recv_tmo;            // 接收超时时间
  int send_tmo;            // 发送超时时间
  sock_wait_t *recv_wait;  // 处理接收等待事件的wait对象
  sock_wait_t *send_wait;  // 处理发送等待事件的wait对象
  sock_wait_t *conn_wait;  // 处理连接等待事件的wait对象

  const sock_ops_t *ops;  // socket操作接口
} sock_t;
// 基础socket对象的方法

net_err_t sock_init(sock_t *sock, int family, int protocol,
                    const sock_ops_t *ops);
void sock_uninit(sock_t *sock);

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

net_err_t sock_req_creat(msg_func_t *msg);
net_err_t sock_req_close(msg_func_t *msg);

// socket发送和接收请求(sendto和recvfrom)的参数结构
typedef struct _sock_io_t {
  void *buf;
  size_t buf_len;
  int flags;
  struct net_sockaddr *sockaddr;
  net_socklen_t sockaddr_len;
  ssize_t ret_len;
} sock_io_t, sock_sendto_t, sock_recvfrom_t;

net_err_t sock_req_sendto(msg_func_t *msg);
net_err_t sock_req_recvfrom(msg_func_t *msg);
net_err_t sock_req_connect(msg_func_t *msg);
net_err_t sock_req_bind(msg_func_t *msg);
net_err_t sock_req_send(msg_func_t *msg);
net_err_t sock_req_recv(msg_func_t *msg);

// socket选项设置请求(setsockopt)的参数结构
typedef struct _sock_opt_t {
  int level;
  int optname;
  const char *optval;
  int optlen;
} sock_opt_t;
net_err_t sock_req_setopt(msg_func_t *msg);

// 用于封装外部线程请求工作线程执行socket相关操作时传递的参数
typedef struct _sock_req_t {
  int sock_fd;        // socket文件描述符
  int wait_tmo;       // 等待超时时间
  sock_wait_t *wait;  // socket等待事件
  union {
    sock_create_t create;  // 创建socket请求的参数
    sock_io_t io;
    sock_opt_t opt;
  };

} sock_req_t;

net_err_t sock_module_init(void);
net_err_t sock_connect(sock_t *sock, const struct net_sockaddr *addr,
                       net_socklen_t addrlen);
net_err_t sock_bind(sock_t *sock, const ipaddr_t *local_ip, const uint16_t local_port);

#endif  // SOCK_H