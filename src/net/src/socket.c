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
 * @brief 打开一个socket对象
 *
 * @param family
 * @param type
 * @param protocol
 * @return int
 */
int net_socket(int family, int type, int protocol) {
  // 封装socket创建请求参数
  sock_req_t sock_req;
  sock_req.create.family = family;
  sock_req.create.type = type;
  sock_req.create.protocol = protocol;
  sock_req.sock_fd = -1;

  // 调用消息队列工作线程执行socket创建请求
  net_err_t err = exmsg_func_exec(sock_req_creat, &sock_req);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_SOCKET, "socket create failed.\n");
    return -1;
  }

  // 返回socket文件描述符
  return sock_req.sock_fd;
}

/**
 * @brief 向指定socket地址发送数据
 * 
 * @param sock 
 * @param buf 
 * @param buf_len 
 * @param flags 
 * @param dest 
 * @param dest_len 
 * @return ssize_t 
 */
ssize_t net_sendto(int sock, const void *buf, size_t buf_len, int flags,
                   const struct net_sockaddr *dest, net_socklen_t dest_len) {
  // 进行参数检查
  if (!buf || !buf_len || !dest || dest_len != sizeof(struct net_sockaddr)) {
    dbg_error(DBG_SOCKET, "sendto param error.\n");
    return -1;
  }
  if (dest->sa_family != AF_INET) {  // 协议栈只支持AF_INET(IPv4)
    dbg_error(DBG_SOCKET, "sendto only support AF_INET.\n");
    return -1;
  }
  // 封装socket发送请求参数
  sock_req_t sock_req;
  sock_req.sock_fd = sock;
  sock_req.io.buf = buf;
  sock_req.io.buf_len = buf_len;
  sock_req.io.flags = flags;
  sock_req.io.sockaddr = (struct net_sockaddr*)dest;
  sock_req.io.sockaddr_len = dest_len;
  sock_req.io.ret_len = 0;

  while (sock_req.io.buf_len > 0) {  // 数据可能需要多次发送
    // 调用消息队列工作线程执行socket发送请求
    net_err_t err = exmsg_func_exec(sock_req_sendto, &sock_req);
    if (err != NET_ERR_OK) {
      dbg_error(DBG_SOCKET, "sendto failed.\n");
      return sock_req.io.ret_len;
    }

    // 还有数据未发送，更新缓冲区位置和大小信息
    sock_req.io.buf = (uint8_t *)buf + sock_req.io.ret_len;
    sock_req.io.buf_len = (size_t)(buf_len - sock_req.io.ret_len);
  }

  // 返回实际发送的数据量(字节量)
  return sock_req.io.ret_len;
}