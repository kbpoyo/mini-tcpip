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

#include "net_plat.h"

#define SOCKET_MAX_CNT 10
static net_socket_t socket_tbl[SOCKET_MAX_CNT];  // socket对象表

/**
 * @brief 初始化socket模块资源
 *
 * @return net_err_t
 */
net_err_t socket_module_init(void) {
  // 初始化socket对象表
  plat_memset(socket_tbl, 0, sizeof(socket_tbl));

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

  // 遍历socket对象表，找到一个空闲状态的socket对象
  for (int i = 0; i < SOCKET_MAX_CNT; i++) {
    net_socket_t *curr = socket_tbl + i;
    if (curr->state == SOCKET_STATE_FREE) {
      // 设置为已使用状态
      curr->state = SOCKET_STATE_USED;
      socket = curr;
      break;
    }
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
}

/**
 * @brief 创建socket对象
 *
 * @param arg
 * @return net_err_t
 */
net_err_t sock_req_creat(msg_func_t *msg) {
  // 获取socket创建请求参数
  sock_req_t *sock_req = (sock_req_t *)msg->arg;

  // 分配一个socket对象
  net_socket_t *socket = socket_alloc();
  if (!socket) {
    dbg_error(DBG_SOCKET, "no free socket object.");
    return NET_ERR_SOCKET;
  }

  // 使用sock_req记录socket对象的文件描述符
  sock_req->sock_fd = socket_get_index(socket);

  return NET_ERR_OK;
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
net_err_t sock_init(sock_t *sock, int family, int protocol, sock_ops_t *ops) {
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

  // 初始化socket对象的挂载节点
  nlist_node_init(&sock->node);

  return NET_ERR_OK;
}