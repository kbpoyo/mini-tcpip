/**
 * @file socket.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 提供给应用程序的socket相关编程接口
 * @version 0.1
 * @date 2024-07-31
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "socket.h"

#include "exmsg.h"
#include "sock.h"

/**
 * @brief 外部接口，打开一个socket对象
 *
 * @param family
 * @param type
 * @param protocol
 * @return int
 */
int net_socket(int family, int type, int protocol) {
  // 封装socket创建请求参数
  sock_req_t sock_req;
  sock_req.wait = (sock_wait_t *)0;
  sock_req.wait_tmo = 0;
  sock_req.create.family = family;
  sock_req.create.type = type;
  sock_req.create.protocol = protocol;
  sock_req.sock_fd = -1;

  // 调用消息队列工作线程执行socket创建请求
  net_err_t err = exmsg_func_exec(sock_req_creat, &sock_req);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_SOCKET, "socket create failed.");
    return -1;
  }

  // 返回socket文件描述符
  return sock_req.sock_fd;
}

/**
 * @brief 外部接口，向指定socket地址发送数据
 *
 * @param sock
 * @param buf
 * @param buf_len
 * @param flags
 * @param dest
 * @param dest_len
 * @return ssize_t
 */
ssize_t net_sendto(int socket, const void *buf, size_t buf_len, int flags,
                   const struct net_sockaddr *dest, net_socklen_t dest_len) {
  // 进行参数检查
  if (socket < 0 || !buf || !buf_len || !dest ||
      dest_len != sizeof(struct net_sockaddr)) {
    dbg_error(DBG_SOCKET, "sendto param error.\n");
    return -1;
  }
  if (dest->sa_family != AF_INET) {  // 协议栈只支持AF_INET(IPv4)
    dbg_error(DBG_SOCKET, "sendto only support AF_INET.\n");
    return -1;
  }
  // 封装socket发送请求参数
  sock_req_t sock_req;
  sock_req.wait = (sock_wait_t *)0;
  sock_req.wait_tmo = 0;
  sock_req.sock_fd = socket;
  sock_req.io.buf = (void *)buf;
  sock_req.io.buf_len = buf_len;
  sock_req.io.flags = flags;
  sock_req.io.sockaddr = (struct net_sockaddr *)dest;
  sock_req.io.sockaddr_len = dest_len;
  sock_req.io.ret_len = 0;

  while (sock_req.io.buf_len > 0) {  // 数据可能需要多次发送
    // 调用消息队列工作线程执行socket发送请求
    net_err_t err = exmsg_func_exec(sock_req_sendto, &sock_req);

    switch (err) {
      case NET_ERR_OK: {  // 发送成功，更新缓冲区位置和大小信息
        sock_req.io.buf = (uint8_t *)buf + sock_req.io.ret_len;
        sock_req.io.buf_len = (size_t)(buf_len - sock_req.io.ret_len);
      } break;
      case NET_ERR_NEEDWAIT: {  // 需要等待内部工作线程执行完毕
        if (sock_wait_enter(sock_req.wait, sock_req.wait_tmo) != NET_ERR_OK) {
          dbg_error(DBG_SOCKET, "socket send wait time out.");
          return sock_req.io.ret_len;
        }
      } break;
      default: {  // 发生其他错误
        dbg_error(DBG_SOCKET, "sendto failed.\n");
        return sock_req.io.ret_len;
      }
    }
  }

  // 返回实际发送的数据量(字节量)
  return sock_req.io.ret_len;
}

/**
 * @brief 外部接口，从指定socket对象接收数据，并记录发送方socket地址
 *
 * @param socket
 * @param buf
 * @param buf_len
 * @param flags
 * @param src
 * @param src_len
 * @return ssize_t
 */
ssize_t net_recvfrom(int socket, void *buf, size_t buf_len, int flags,
                     struct net_sockaddr *src, net_socklen_t *src_len) {
  // 进行参数检查
  if (socket < 0 || !buf || !buf_len || !src || !src_len) {
    dbg_error(DBG_SOCKET, "recvfrom param error.\n");
    return -1;
  }

  while (1) {
    // 封装socket接收请求参数
    sock_req_t sock_req;
    sock_req.wait =
        (sock_wait_t *)0;  // wait对象，用来等待内部工作线程的接收结果
    sock_req.wait_tmo = 0;
    sock_req.sock_fd = socket;
    sock_req.io.buf = buf;
    sock_req.io.buf_len = buf_len;
    sock_req.io.flags = flags;
    sock_req.io.sockaddr = src;
    sock_req.io.sockaddr_len = 0;
    sock_req.io.ret_len = 0;

    // 调用消息队列工作线程执行socket接收请求
    net_err_t err = exmsg_func_exec(sock_req_recvfrom, &sock_req);
    switch (err) {
      case NET_ERR_OK: {
        // 记录发送方socket地址大小
        *src_len = sock_req.io.sockaddr_len;
        // 返回实际接收的数据量(字节量)， 若返回-1表示接收失败
        return sock_req.io.ret_len > 0 ? sock_req.io.ret_len : -1;
      } break;
      case NET_ERR_NEEDWAIT: {  // 需要等待内部工作线程执行完毕
        if (sock_wait_enter(sock_req.wait, sock_req.wait_tmo) != NET_ERR_OK) {
          dbg_error(DBG_SOCKET, "socket wait error.");
          return -1;
        }
      } break;
      default: {  // 发生其他错误
        dbg_error(DBG_SOCKET, "recvfrom failed.\n");
        return -1;
      }
    }
  }
}

int net_close(int socket) {
  // 封装socket关闭请求参数
  sock_req_t sock_req;
  sock_req.wait = (sock_wait_t *)0;
  sock_req.wait_tmo = 0;
  sock_req.sock_fd = socket;

  while (1) {
    // 调用消息队列工作线程执行socket关闭请求
    net_err_t err = exmsg_func_exec(sock_req_close, &sock_req);
    switch (err) {
      case NET_ERR_OK: {  // 关闭成功
        return 0;
      } break;
      case NET_ERR_NEEDWAIT: {  // 需要等待内部工作线程执行完毕(TCP协议需要等待对端关闭)
        net_err_t err = sock_wait_enter(sock_req.wait, sock_req.wait_tmo);
        if (err != NET_ERR_OK && err != NET_ERR_TCP_CLOSE) {
          dbg_error(DBG_SOCKET, "socket close wait time out.");
          return -1;
        }
        continue;  // 等待结束，重新执行关闭请求
      } break;
      default: {  // 发生其他错误
        dbg_error(DBG_SOCKET, "socket close failed.\n");
        return -1;
      } break;
    }
  }
}

/**
 * @brief 外部接口, 根据选项级别(level)和对应的选项类型(optname)来设置socket选项
 *
 * @param socket
 * @param level
 * @param optname
 * @param optval
 * @param optlen
 * @return int
 */
int net_setsockopt(int socket, int level, int optname, const char *optval,
                   int optlen) {
  // 进行参数检查
  if (socket < 0 || !optval || !optlen) {
    dbg_error(DBG_SOCKET, "setsockopt param error.\n");
    return -1;
  }

  // 封装socket接收请求参数
  sock_req_t sock_req;
  sock_req.wait = (sock_wait_t *)0;  // wait对象，用来等待内部工作线程的接收结果
  sock_req.wait_tmo = 0;
  sock_req.sock_fd = socket;
  sock_req.opt.level = level;
  sock_req.opt.optname = optname;
  sock_req.opt.optval = optval;
  sock_req.opt.optlen = optlen;

  // 调用消息队列工作线程执行socket接收请求
  net_err_t err = exmsg_func_exec(sock_req_setopt, &sock_req);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_SOCKET, "setsockopt failed.\n");
    return -1;
  }

  return 0;
}

/**
 * @brief 外部接口， socket连接指定地址, 只与该远端地址通信
 *
 * @param socket
 * @param addr
 * @param addrlen
 * @return int
 */
int net_connect(int socket, const struct net_sockaddr *addr,
                net_socklen_t addrlen) {
  // 进行参数检查
  if (socket < 0 || !addr || addrlen != sizeof(struct net_sockaddr)) {
    dbg_error(DBG_SOCKET, "connect param error.\n");
    return -1;
  }
  struct net_sockaddr_in *addr_in = (struct net_sockaddr_in *)addr;
  if (addr_in->sin_family != AF_INET) {
    dbg_error(DBG_SOCKET, "connect only support AF_INET(IPV4).\n");
    return -1;
  }
  if (addr_in->sin_port < 0 || addr_in->sin_addr.s_addr == 0) {
    dbg_error(DBG_SOCKET, "connect address error.\n");
    return -1;
  }

  // 封装socket接收请求参数
  sock_req_t sock_req;
  sock_req.wait = (sock_wait_t *)0;  // wait对象，用来等待内部工作线程的接收结果
  sock_req.wait_tmo = 0;
  sock_req.sock_fd = socket;
  sock_req.io.sockaddr = (struct net_sockaddr *)addr;
  sock_req.io.sockaddr_len = addrlen;

  // 调用消息队列工作线程执行socket连接请求
  net_err_t err = exmsg_func_exec(sock_req_connect, &sock_req);
  switch (err) {
    case NET_ERR_OK: {  // 连接成功
      return 0;
    } break;
    case NET_ERR_NEEDWAIT: {  // 需要等待内部工作线程执行完毕(TCP协议需要等待三次握手完成连接)
      if (sock_wait_enter(sock_req.wait, sock_req.wait_tmo) != NET_ERR_OK) {
        dbg_error(DBG_SOCKET, "socket connect wait time out.");
        return -1;
      }
      return 0;
    } break;
    default: {  // 发生其他错误
      dbg_error(DBG_SOCKET, "socket connect failed.\n");
      return -1;
    } break;
  }
}

/**
 * @brief 外部接口， socket绑定指定地址, 指定本地ip地址和端口
 *
 * @param socket
 * @param addr
 * @param addrlen
 * @return int
 */
int net_bind(int socket, const struct net_sockaddr *addr,
             net_socklen_t addrlen) {
  // 进行参数检查
  if (socket < 0 || !addr || addrlen != sizeof(struct net_sockaddr)) {
    dbg_error(DBG_SOCKET, "bind param error.\n");
    return -1;
  }
  struct net_sockaddr_in *addr_in = (struct net_sockaddr_in *)addr;
  if (addr_in->sin_family != AF_INET) {
    dbg_error(DBG_SOCKET, "bind only support AF_INET(IPV4).\n");
    return -1;
  }
  if (addr_in->sin_port <= 0) {
    dbg_error(DBG_SOCKET, "bind port error, port <= 0.\n");
    return -1;
  }

  // 封装socket接收请求参数
  sock_req_t sock_req;
  sock_req.wait = (sock_wait_t *)0;  // wait对象，用来等待内部工作线程的接收结果
  sock_req.wait_tmo = 0;
  sock_req.sock_fd = socket;
  sock_req.io.sockaddr = (struct net_sockaddr *)addr;
  sock_req.io.sockaddr_len = addrlen;

  // 调用消息队列工作线程执行socket连接请求
  net_err_t err = exmsg_func_exec(sock_req_bind, &sock_req);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_SOCKET, "bind failed.\n");
    return -1;
  }

  return 0;
}

/**
 * @brief 通过socket发送数据，socket已经调用connect()函数连接到远端地址
 *
 * @param socket
 * @param buf
 * @param buf_len
 * @param flags
 * @return ssize_t
 */
ssize_t net_send(int socket, const void *buf, size_t buf_len, int flags) {
  // 进行参数检查
  if (socket < 0 || !buf || !buf_len) {
    dbg_error(DBG_SOCKET, "sendto param error.\n");
    return -1;
  }

  // 封装socket发送请求参数
  sock_req_t sock_req;
  sock_req.wait = (sock_wait_t *)0;
  sock_req.wait_tmo = 0;
  sock_req.sock_fd = socket;
  sock_req.io.buf = (void *)buf;
  sock_req.io.buf_len = buf_len;
  sock_req.io.flags = flags;
  sock_req.io.ret_len = 0;

  while (sock_req.io.buf_len > 0) {  // 数据可能需要多次发送
    // 调用消息队列工作线程执行socket发送请求
    net_err_t err = exmsg_func_exec(sock_req_send, &sock_req);

    switch (err) {
      case NET_ERR_OK: {  // 发送成功，更新缓冲区位置和大小信息
        sock_req.io.buf = (uint8_t *)buf + sock_req.io.ret_len;
        sock_req.io.buf_len = (size_t)(buf_len - sock_req.io.ret_len);
      } break;
      case NET_ERR_NEEDWAIT: {  // 需要等待内部工作线程执行完毕
        if (sock_wait_enter(sock_req.wait, sock_req.wait_tmo) != NET_ERR_OK) {
          dbg_error(DBG_SOCKET, "socket send wait time out.");
          return sock_req.io.ret_len;
        }
      } break;
      default: {  // 发生其他错误
        dbg_error(DBG_SOCKET, "send failed.\n");
        return sock_req.io.ret_len;
      }
    }
  }

  // 返回实际发送的数据量(字节量)
  return sock_req.io.ret_len;
}

/**
 * @brief
 * 外部接口，通过socket接收数据，socket已经调用connect()函数连接到远端地址
 *
 * @param socket
 * @param buf
 * @param buf_len
 * @param flags
 * @return ssize_t
 */
ssize_t net_recv(int socket, void *buf, size_t buf_len, int flags) {
  // 进行参数检查
  if (socket < 0 || !buf || !buf_len) {
    dbg_error(DBG_SOCKET, "recv param error.\n");
    return -1;
  }

  while (1) {
    // 封装socket接收请求参数
    sock_req_t sock_req;
    sock_req.wait =
        (sock_wait_t *)0;  // wait对象，用来等待内部工作线程的接收结果
    sock_req.wait_tmo = 0;
    sock_req.sock_fd = socket;
    sock_req.io.buf = buf;
    sock_req.io.buf_len = buf_len;
    sock_req.io.flags = flags;
    sock_req.io.ret_len = 0;

    // 调用消息队列工作线程执行socket接收请求
    net_err_t err = exmsg_func_exec(sock_req_recv, &sock_req);
    switch (err) {
      case NET_ERR_OK: {
        // 返回实际接收的数据量(字节量)， 若返回-1表示接收失败
        return sock_req.io.ret_len > 0 ? sock_req.io.ret_len : -1;
      } break;
      case NET_ERR_NEEDWAIT: {  // 需要等待内部工作线程执行完毕
        if (sock_wait_enter(sock_req.wait, sock_req.wait_tmo) != NET_ERR_OK) {
          dbg_error(DBG_SOCKET, "socket wait error.");
          return -1;
        }
      } break;
      default: {  // 发生其他错误
        dbg_error(DBG_SOCKET, "recvfrom failed.\n");
        return -1;
      }
    }
  }
}
