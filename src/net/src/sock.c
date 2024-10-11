/**
 * @file sock.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 协议栈内部socket相关数据结构及方法
 * @version 0.1
 * @date 2024-07-31
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "sock.h"

#include "mblock.h"
#include "net_plat.h"
#include "route.h"
#include "sock_raw.h"
#include "tools.h"
#include "udp.h"
#include "tcp.h"

// 定义协议栈可用的socket对象最大数量
#define SOCKET_MAX_CNT (SOCKRAW_MAXCNT + UDP_MAXCNT + TCP_MAXCNT)

static net_socket_t socket_tbl[SOCKET_MAX_CNT];  // socket对象表
static mblock_t socket_mblock;  // socket对象内存块管理对象
static nlist_t socket_list;     // 挂载已分配的socket对象链表

/**
 * @brief 初始化socket等待事件对象
 *
 * @param wait 等待事件对象
 * @return net_err_t
 */
net_err_t sock_wait_init(sock_wait_t *wait) {
  wait->wait_event_cnt = 0;
  wait->error = NET_ERR_OK;
  wait->sem = sys_sem_create(0);

  return wait->sem == SYS_SEM_INVALID ? NET_ERR_SOCKET : NET_ERR_OK;
}

/**
 * @brief 销毁socket等待事件对象
 *
 * @param wait
 */
void sock_wait_destroy(sock_wait_t *wait) {
  if (wait) {
    if (wait->sem != SYS_SEM_INVALID) {
      sys_sem_free(wait->sem);
      wait->sem = SYS_SEM_INVALID;
    }
  }
}
/**
 * @brief 为方法请求对象添加一个wait事件
 *
 * @param wait
 * @param req
 * @param tmo
 * @return net_err_t
 */
net_err_t sock_wait_add(sock_wait_t *wait, int tmo, struct _sock_req_t *req) {
  // 增加等待事件数
  wait->wait_event_cnt++;
  // 方法请求对象记录wait对象和对应的超时时间
  req->wait = wait;
  req->wait_tmo = tmo;

  return NET_ERR_OK;
}

/**
 * @brief 外部线程通过wait对象进行事件等待
 *
 * @param wait
 * @param tmo
 * @return net_err_t
 */
net_err_t sock_wait_enter(sock_wait_t *wait, int tmo) {
  if (sys_sem_wait(wait->sem, tmo) < 0) {  // 等待获取信号量
    return NET_ERR_TIMEOUT;
  }
  // 等待成功，返回错误码
  return wait->error;
}

/**
 * @brief 内部工作线程在完成事件后，检查wait对象上是否有线程等待，若有则将其唤醒
 * 并传递线程完成情况即错误码
 *
 * @param wait
 * @param error
 */
void sock_wait_leave(sock_wait_t *wait, net_err_t error) {
  if (wait->wait_event_cnt > 0) {
    wait->wait_event_cnt--;
    sys_sem_notify(wait->sem);
    wait->error = error;
  }
}

/**
 * @brief 根据等待事件的类型，唤醒等待在基类sock的wait对象上的线程
 *
 * @param sock
 * @param wait_type
 * @param err
 */
void sock_wakeup(sock_t *sock, int wait_type, int err) {
  if (wait_type & SOCK_WAIT_CONN) {
    sock_wait_leave(sock->conn_wait, err);
  }
  if (wait_type & SOCK_WAIT_READ) {
    sock_wait_leave(sock->recv_wait, err);
  }
  if (wait_type & SOCK_WAIT_WRITE) {
    sock_wait_leave(sock->send_wait, err);
  }
}

/**
 * @brief 初始化socket模块资源
 *
 * @return net_err_t
 */
net_err_t sock_module_init(void) {
  // 初始化socket对象内存块管理对象
  // 只由工作线程访问，不需要加锁
  mblock_init(&socket_mblock, socket_tbl, sizeof(net_socket_t), SOCKET_MAX_CNT,
              NLOCKER_NONE);

  // 初始化socket对象挂载链表
  nlist_init(&socket_list);

  return NET_ERR_OK;
}

/**
 * @brief 获取socket对象在socket对象表中的索引
 *
 * @param socket
 * @return int
 */
static int socket_get_index(net_socket_t *socket) {
  // 相同类型的指针相减的结果为其地址差值除以类型大小
  return (int)(socket - socket_tbl);
}

/**
 * @brief 通过索引获取socket对象
 *
 * @param index
 * @return net_socket_t*
 */
static net_socket_t *socket_by_index(int index) {
  if (index < 0 || index >= SOCKET_MAX_CNT) {
    return (net_socket_t *)0;
  }
  return socket_tbl + index;
}

/**
 * @brief 从socket对象表中分配一个空闲状态的socket对象
 *
 * @return net_socket_t*
 */
static net_socket_t *socket_alloc(void) {
  net_socket_t *socket = (net_socket_t *)0;

  // 从socket对象内存块管理对象中分配一个socket对象
  socket = (net_socket_t *)mblock_alloc(&socket_mblock, -1);
  if (socket) {
    // 设置为已使用状态
    socket->state = SOCKET_STATE_USED;
  }

  return socket;
}

/**
 * @brief 释放一个socket对象
 *
 * @param socket
 */
static void socket_free(net_socket_t *socket) {
  // 设置为空闲状态
  socket->state = SOCKET_STATE_FREE;

  // 从socket对象链表中移除
  nlist_remove(&socket_list, &socket->node);

  // 释放socket对象内存块
  mblock_free(&socket_mblock, socket);
}

/**
 * @brief 外部应用请求创建socket对象
 *
 * @param msg 消息类型：函数执行请求
 * @return net_err_t
 */
net_err_t sock_req_creat(msg_func_t *msg) {
  // 定义一个sock方法分配结构表，实现基础socket的静态多态效果
  static const struct sock_type_t {
    int default_family;
    int default_protocol;
    sock_t *(*create)(int family, int protocol);
  } sock_type_tbl[] = {
      [SOCK_RAW] = {.default_protocol = IPPROTO_ICMP, .create = sockraw_create},
      [SOCK_DGRAM] = {.default_protocol = IPPROTO_UDP, .create = udp_create},
      [SOCK_STREAM] = {.default_protocol = IPPROTO_TCP, .create = tcp_create},
  };

  // 获取socket创建请求参数
  sock_req_t *sock_req = (sock_req_t *)msg->arg;
  sock_create_t *create = &sock_req->create;

  // 根据指定类型获取socket对象方法的创建方法
  if (create->type < 0 ||
      create->type >= (sizeof(sock_type_tbl) / sizeof(sock_type_tbl[0]))) {
    dbg_error(DBG_SOCKET, "invalid socket type.");
    return NET_ERR_SOCKET;
  }
  const struct sock_type_t *sock_type = &sock_type_tbl[create->type];

  // 若没有指定上层协议，则使用默认协议
  create->protocol =
      create->protocol ? create->protocol : sock_type->default_protocol;

  // 分配一个socket对象, 用于封装即将打开的内部sock对象, 并挂载到socket对象链表
  net_socket_t *socket = socket_alloc();
  if (!socket) {
    dbg_error(DBG_SOCKET, "no free socket object.");
    return NET_ERR_SOCKET;
  }
  nlist_insert_last(&socket_list, &socket->node);

  // 调用指定类型的sock对象创建方法, 创建sock对象, 并记录到socket对象
  sock_t *sock = sock_type->create(create->family, create->protocol);
  if (!sock) {  // 创建失败
    dbg_error(DBG_SOCKET, "create socket failed.");
    socket_free(socket);  // 释放socket对象
    return NET_ERR_SOCKET;
  }
  socket->sock = sock;

  // 使用sock_req记录socket对象的文件描述符
  sock_req->sock_fd = socket_get_index(socket);

  return NET_ERR_OK;
}

/**
 * @brief 外部应用请求发送数据
 *
 * @param msg
 * @return net_err_t
 */
net_err_t sock_req_sendto(msg_func_t *msg) {
  // 获取socket发送请求参数
  sock_req_t *sock_req = (sock_req_t *)msg->arg;
  sock_io_t *io = &sock_req->io;

  // 获取封装socket对象, 并获取其基类对象
  net_socket_t *socket = socket_by_index(sock_req->sock_fd);
  if (!socket) {
    dbg_error(DBG_SOCKET, "invalid socket fd.");
    return NET_ERR_SOCKET;
  }
  sock_t *sock = socket->sock;

  // 调用socket对象的发送方法，进行静态多态调用
  if (!sock->ops->sendto) {
    dbg_error(DBG_SOCKET, "socket sendto not supported.");
    return NET_ERR_SOCKET;
  }
  net_err_t err =
      socket->sock->ops->sendto(socket->sock, io->buf, io->buf_len, io->flags,
                                io->sockaddr, io->sockaddr_len, &io->ret_len);
  if (err == NET_ERR_NEEDWAIT) {  // 通知外部线程等待执行结果
    if (sock->send_wait) {
      // 为方法请求对象添加一个wait对象, 使用该wait对象阻塞外部线程
      sock_wait_add(sock->send_wait, sock->send_tmo, sock_req);
    } else {
      dbg_error(DBG_SOCKET, "socket don't have send wait obj.");
      return NET_ERR_SOCKET;
    }
  }

  return err;
}

/**
 * @brief 内部接口，通过socket对象发送数据，且socket对象已连接远端地址
 *
 * @param msg
 * @return net_err_t
 */
net_err_t sock_req_send(msg_func_t *msg) {
  // 获取socket发送请求参数
  sock_req_t *sock_req = (sock_req_t *)msg->arg;
  sock_io_t *io = &sock_req->io;

  // 获取封装socket对象, 并获取其基类对象
  net_socket_t *socket = socket_by_index(sock_req->sock_fd);
  if (!socket) {
    dbg_error(DBG_SOCKET, "invalid socket fd.");
    return NET_ERR_SOCKET;
  }
  sock_t *sock = socket->sock;

  // 调用socket对象的发送方法，进行静态多态调用
  if (!sock->ops->send) {
    dbg_error(DBG_SOCKET, "socket send not supported.");
    return NET_ERR_SOCKET;
  }
  net_err_t err = socket->sock->ops->send(socket->sock, io->buf, io->buf_len,
                                          io->flags, &io->ret_len);
  if (err == NET_ERR_NEEDWAIT) {  // 通知外部线程等待执行结果
    if (sock->send_wait) {
      // 为方法请求对象添加一个wait对象, 使用该wait对象阻塞外部线程
      sock_wait_add(sock->send_wait, sock->send_tmo, sock_req);
    } else {
      dbg_error(DBG_SOCKET, "socket don't have send wait obj.");
      return NET_ERR_SOCKET;
    }
  }

  return err;
}

/**
 * @brief 外部应用请求接收数据
 *
 * @param msg
 * @return net_err_t
 */
net_err_t sock_req_recvfrom(msg_func_t *msg) {
  // 获取socket发送请求参数
  sock_req_t *sock_req = (sock_req_t *)msg->arg;
  sock_io_t *io = &sock_req->io;

  // 获取socket对象
  net_socket_t *socket = socket_by_index(sock_req->sock_fd);
  if (!socket) {
    dbg_error(DBG_SOCKET, "invalid socket fd.");
    return NET_ERR_SOCKET;
  }

  // 获取socket基类对象
  sock_t *sock = socket->sock;

  // 调用socket对象的recvfrom方法，完成静态多态调用
  if (!sock->ops->recvfrom) {
    dbg_error(DBG_SOCKET, "socket recvfrom not supported.");
    return NET_ERR_SOCKET;
  }
  net_err_t err = socket->sock->ops->recvfrom(
      socket->sock, io->buf, io->buf_len, io->flags, io->sockaddr,
      &io->sockaddr_len, &io->ret_len);

  if (err == NET_ERR_NEEDWAIT) {  // 通知外部线程等待执行结果
    if (sock->recv_wait) {
      // 为方法请求对象添加一个wait对象, 使用该wait对象阻塞外部线程
      sock_wait_add(sock->recv_wait, sock->recv_tmo, sock_req);
    } else {
      dbg_error(DBG_SOCKET, "socket don't have recv wait obj.");
      return NET_ERR_SOCKET;
    }
  }

  return err;
}

/**
 * @brief 内部接口，通过socket对象接收数据，且socket对象已连接远端地址
 *
 * @param msg
 * @return net_err_t
 */
net_err_t sock_req_recv(msg_func_t *msg) {
  // 获取socket发送请求参数
  sock_req_t *sock_req = (sock_req_t *)msg->arg;
  sock_io_t *io = &sock_req->io;

  // 获取封装socket对象, 并获取其基类对象
  net_socket_t *socket = socket_by_index(sock_req->sock_fd);
  if (!socket) {
    dbg_error(DBG_SOCKET, "invalid socket fd.");
    return NET_ERR_SOCKET;
  }
  sock_t *sock = socket->sock;

  // 调用socket对象的发送方法，进行静态多态调用
  if (!sock->ops->recv) {
    dbg_error(DBG_SOCKET, "socket recv not supported.");
    return NET_ERR_SOCKET;
  }
  net_err_t err = socket->sock->ops->recv(socket->sock, io->buf, io->buf_len,
                                          io->flags, &io->ret_len);
  if (err == NET_ERR_NEEDWAIT) {  // 通知外部线程等待执行结果
    if (sock->recv_wait) {
      // 为方法请求对象添加一个wait对象, 使用该wait对象阻塞外部线程
      sock_wait_add(sock->recv_wait, sock->recv_tmo, sock_req);
    } else {
      dbg_error(DBG_SOCKET, "socket don't have recv wait obj.");
      return NET_ERR_SOCKET;
    }
  }

  return err;
}

/**
 * @brief 外部应用请求设置socket选项
 *
 * @param msg
 * @return net_err_t
 */
net_err_t sock_req_setopt(msg_func_t *msg) {
  // 获取socket选项设置请求参数
  sock_req_t *sock_req = (sock_req_t *)msg->arg;
  sock_opt_t *opt = &sock_req->opt;

  // 获取封装的socket对象
  net_socket_t *socket = socket_by_index(sock_req->sock_fd);
  if (!socket) {
    dbg_error(DBG_SOCKET, "invalid socket fd.");
    return NET_ERR_SOCKET;
  }

  // 获取socket基类对象
  sock_t *sock = socket->sock;

  // 调用socket对象的setsockopt方法，完成静态多态调用
  if (!sock->ops->setopt) {
    dbg_error(DBG_SOCKET, "socket setsockopt not supported.");
    return NET_ERR_SOCKET;
  }
  return sock->ops->setopt(sock, opt->level, opt->optname, opt->optval,
                           opt->optlen);
}

/**
 * @brief 内部接口，连接指定socket对象，只与connect连接的远端地址通信
 *
 * @param msg
 * @return net_err_t
 */
net_err_t sock_req_connect(msg_func_t *msg) {
  // 获取socket选项设置请求参数
  sock_req_t *sock_req = (sock_req_t *)msg->arg;
  const struct net_sockaddr *addr = sock_req->io.sockaddr;
  net_socklen_t addr_len = sock_req->io.sockaddr_len;

  // 获取封装的socket对象
  net_socket_t *socket = socket_by_index(sock_req->sock_fd);
  if (!socket) {
    dbg_error(DBG_SOCKET, "invalid socket fd.");
    return NET_ERR_SOCKET;
  }

  // 获取socket基类对象, 并完成静态多态调用
  sock_t *sock = socket->sock;
  if (!sock->ops->connect) {
    dbg_error(DBG_SOCKET, "socket connect not supported.");
    return NET_ERR_SOCKET;
  }
  net_err_t err = sock->ops->connect(sock, addr, addr_len);
  switch (err) {        // 处理连接结果
    case NET_ERR_OK: {  // 连接成功
      return NET_ERR_OK;
    } break;
    case NET_ERR_NEEDWAIT: {  // 通知外部线程等待执行结果
      if (sock->conn_wait) {
        // 为方法请求对象添加一个wait对象, 使用该wait对象阻塞外部线程
        sock_wait_add(sock->conn_wait, sock->recv_tmo, sock_req);
      } else {
        dbg_error(DBG_SOCKET, "socket don't have connect wait obj.");
        return NET_ERR_SOCKET;
      }
    } break;
    default: {  // 连接失败
      dbg_error(DBG_SOCKET, "socket connect failed.");
      return err;
    } break;
  }
}

/**
 * @brief 内部接口，socket绑定本地ip地址与端口
 *
 * @param msg
 * @return net_err_t
 */
net_err_t sock_req_bind(msg_func_t *msg) {
  // 获取socket选项设置请求参数
  sock_req_t *sock_req = (sock_req_t *)msg->arg;
  const struct net_sockaddr *addr = sock_req->io.sockaddr;
  net_socklen_t addr_len = sock_req->io.sockaddr_len;

  // 获取封装的socket对象
  net_socket_t *socket = socket_by_index(sock_req->sock_fd);
  if (!socket) {
    dbg_error(DBG_SOCKET, "invalid socket fd.");
    return NET_ERR_SOCKET;
  }

  // 获取socket基类对象, 并完成静态多态调用
  sock_t *sock = socket->sock;
  if (!sock->ops->bind) {
    dbg_error(DBG_SOCKET, "socket bind not supported.");
    return NET_ERR_SOCKET;
  }

  return sock->ops->bind(sock, addr, addr_len);
}

/**
 * @brief 初始化基础socket对象(sock)
 *
 * @param sock 基础socket对象
 * @param family 网络层地址族(AF_INET, AF_INET6):(IPv4, IPv6)
 * @param type
 * @param protocol 上层协议(udp,tcp)或icmp协议
 * @param ops 基础socket操作接口
 * @return net_err_t
 */
net_err_t sock_init(sock_t *sock, int family, int protocol,
                    const sock_ops_t *ops) {
  // 连接的网络层和传输层的协议信息
  sock->family = family;
  sock->protocol = protocol;
  sock->ops = ops;

  // 设置连接的本地和远端IP地址和端口
  ipaddr_set_any(&sock->local_ip);
  ipaddr_set_any(&sock->remote_ip);
  sock->local_port = 0;
  sock->remote_port = 0;

  // 设置连接的错误码和超时时间
  sock->err_code = NET_ERR_OK;
  sock->recv_tmo = 0;
  sock->send_tmo = 0;

  // 设置wait对象
  sock->recv_wait = (sock_wait_t *)0;
  sock->send_wait = (sock_wait_t *)0;
  sock->conn_wait = (sock_wait_t *)0;

  // 初始化socket对象的挂载节点
  nlist_node_init(&sock->node);

  return NET_ERR_OK;
}

/**
 * @brief 销毁一个已初始化的基类socket对象，以释放其持有的资源
 *
 * @param sock
 */
void sock_uninit(sock_t *sock) {
  // 销毁socket对象的wait对象
  if (sock->recv_wait) {
    sock_wait_destroy(sock->recv_wait);
    sock->recv_wait = (sock_wait_t *)0;
  }
  if (sock->send_wait) {
    sock_wait_destroy(sock->send_wait);
    sock->send_wait = (sock_wait_t *)0;
  }
  if (sock->conn_wait) {
    sock_wait_destroy(sock->conn_wait);
    sock->conn_wait = (sock_wait_t *)0;
  }
}

/**
 * @brief 关闭一个socket对象
 *
 * @param msg
 * @return net_err_t
 */
net_err_t sock_req_close(msg_func_t *msg) {
  // 获取socket关闭请求参数
  sock_req_t *sock_req = (sock_req_t *)msg->arg;

  // 获取socket对象
  net_socket_t *socket = socket_by_index(sock_req->sock_fd);
  if (!socket) {
    dbg_error(DBG_SOCKET, "invalid socket fd.");
    return NET_ERR_SOCKET;
  }

  // 获取socket基类对象
  sock_t *sock = socket->sock;

  // 调用socket对象的destroy方法，完成静态多态调用
  if (!sock->ops->close) {
    dbg_error(DBG_SOCKET, "socket destroy not supported.");
    return NET_ERR_SOCKET;
  }
  net_err_t err = sock->ops->close(sock);
  // TODO: 暂时不处理NET_ERR_NEEDWAIT错误
  if (err != NET_ERR_OK) {
    dbg_error(DBG_SOCKET, "socket close failed.\n");
    return err;
  }

  // 释放socket对象
  socket_free(socket);

  return NET_ERR_OK;
}

/**
 * @brief 将addr中的ip和端口信息记录到sock对象中
 *
 * @param sock
 * @param addr
 * @param addr_len
 * @return net_err_t
 */
net_err_t sock_connect(sock_t *sock, const struct net_sockaddr *addr,
                       net_socklen_t addr_len) {
  // 将远端地址信息拷贝到sock对象中
  const struct net_sockaddr_in *remote_addr =
      (const struct net_sockaddr_in *)addr;
  ipaddr_from_bytes(&sock->remote_ip, remote_addr->sin_addr.s_addr_bytes);
  sock->remote_port = net_ntohs(remote_addr->sin_port);
  return NET_ERR_OK;
}

/**
 * @brief 将sock对象绑定到本地地址
 *
 * @param sock
 * @param addr
 * @param addr_len
 * @return net_err_t
 */
net_err_t sock_bind(sock_t *sock, const ipaddr_t *local_ip, const uint16_t local_port) {
  // 若指定了ip地址，则判断其是否在本地网络接口中
  if (!ipaddr_is_any(local_ip)) {
    route_entry_t *rt_entry = route_find(local_ip);
    if (!rt_entry || !ipaddr_is_equal(&rt_entry->netif->ipaddr, local_ip)) {
      dbg_error(DBG_SOCKET, "local ip not found in local netif.\n");
      return NET_ERR_SOCKET;
    }
  }

  // 指定的ip地址在本地网络接口中，将其绑定到sock对象中
  // 若未指定ip地址(local_ip 为空)，则由ip层在发送时根据目的地址自动选择
  ipaddr_copy(&sock->local_ip, local_ip);
  sock->local_port = local_port;

  return NET_ERR_OK;
}

/***********************************************************************************************************
 * @brief
 * 基类sock的通用接口实现，后续派生类可直接继承方法，不用再独立实现。
 *
 ***********************************************************************************************************


 /**
 * @brief 基类sock的setsockopt方法实现，后续派生类可直接继承该方法,
 * 不用再独立实现。
 *
 * @param sock
 * @param level
 * @param optname
 * @param optval
 * @param optlen
 * @return int
 */
net_err_t sock_setopt(sock_t *sock, int level, int optname, const char *optval,
                      int optlen) {
  // 判断选项级别是设置在哪一层
  if (level != SOL_SOCKET) {  // TODO: 暂时只支持在socket层设置选项
    dbg_error(DBG_SOCKET, "invalid socket option level.\n");
    return NET_ERR_SOCKET;
  }

  // 根据选项类型进行处理
  switch (optname) {
    case SO_RCVTIMEO: {                            // 设置接收超时时间
      if (optlen != sizeof(struct net_timeval)) {  // 选项参数类型大小不匹配
        dbg_error(DBG_SOCKET, "invalid socket option value.\n");
        return NET_ERR_SOCKET;
      }
      struct net_timeval *tmo = (struct net_timeval *)optval;
      sock->recv_tmo = tmo->tv_sec * 1000 + tmo->tv_usec / 1000;
    } break;

    case SO_SNDTIMEO: {                            // 设置发送超时时间
      if (optlen != sizeof(struct net_timeval)) {  // 选项参数类型大小不匹配
        dbg_error(DBG_SOCKET, "invalid socket option value.\n");
        return NET_ERR_SOCKET;
      }
      struct net_timeval *tmo = (struct net_timeval *)optval;
      sock->send_tmo = tmo->tv_sec * 1000 + tmo->tv_usec / 1000;
    } break;

    default: {
      dbg_error(DBG_SOCKET, "invalid socket option name.\n");
      return NET_ERR_SOCKET;
    } break;
  }

  return NET_ERR_OK;
}

/**
 * @brief 基类的send方法实现，后续派生类可直接继承该方法
 *
 * @param sock
 * @param buf
 * @param buf_len
 * @param flags
 * @param ret_send_len
 * @return neterr_t
 */
net_err_t sock_send(sock_t *sock, const void *buf, size_t buf_len, int flags,
                    ssize_t *ret_send_len) {
  struct net_sockaddr_in addr;
  addr.sin_family = sock->family;
  addr.sin_port = net_htons(sock->remote_port);
  ipaddr_to_bytes(&sock->remote_ip, addr.sin_addr.s_addr_bytes);

  return sock->ops->sendto(sock, buf, buf_len, flags,
                           (const struct net_sockaddr *)&addr,
                           sizeof(struct net_sockaddr_in), ret_send_len);
}

/**
 * @brief 基类的recv方法实现，后续派生类可直接继承该方法
 *
 * @param sock
 * @param buf
 * @param buf_len
 * @param flags
 * @param ret_recv_len
 * @return neterr_t
 */
net_err_t sock_recv(sock_t *sock, void *buf, size_t buf_len, int flags,
                    ssize_t *ret_recv_len) {
  return sock->ops->recvfrom(sock, buf, buf_len, flags, 0, 0, ret_recv_len);
}