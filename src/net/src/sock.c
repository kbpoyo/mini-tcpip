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
#include "sock_raw.h"

// 定义协议栈可用的socket对象最大数量
#define SOCKET_MAX_CNT (SOCKRAW_MAX_CNT)

static net_socket_t socket_tbl[SOCKET_MAX_CNT];  // socket对象表
static mblock_t socket_mblock;  // socket对象内存块管理对象
static nlist_t socket_list;     // 挂载已分配的socket对象链表

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

  // // 遍历socket对象表，找到一个空闲状态的socket对象
  // for (int i = 0; i < SOCKET_MAX_CNT; i++) {
  //   net_socket_t *curr = socket_tbl + i;
  //   if (curr->state == SOCKET_STATE_FREE) {
  //     // 设置为已使用状态
  //     curr->state = SOCKET_STATE_USED;
  //     socket = curr;
  //     break;
  //   }
  // }

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
  };

  // 获取socket创建请求参数
  sock_req_t *sock_req = (sock_req_t *)msg->arg;
  sock_create_t *create = &sock_req->create;

  // 获取指定类型的socket对象方法
  if (create->type < 0 ||
      create->type >= (sizeof(sock_type_tbl) / sizeof(sock_type_tbl[0]))) {
    dbg_error(DBG_SOCKET, "invalid socket type.");
    return NET_ERR_SOCKET;
  }
  const struct sock_type_t *sock_type = &sock_type_tbl[create->type];

  // 若没有指定上层协议，则使用默认协议
  create->protocol =
      create->protocol ? create->protocol : sock_type->default_protocol;

  // 分配一个socket对象, 用于封装即将打开的内部sock对象
  net_socket_t *socket = socket_alloc();
  if (!socket) {
    dbg_error(DBG_SOCKET, "no free socket object.");
    return NET_ERR_SOCKET;
  }
  // 将socket对象挂载到socket对象链表
  nlist_insert_last(&socket_list, &socket->node);

  // 调用指定类型的sock对象创建方法, 创建sock对象
  sock_t *sock = sock_type->create(create->family, create->protocol);
  if (!sock) {  // 创建失败
    dbg_error(DBG_SOCKET, "create socket failed.");
    socket_free(socket);  // 释放socket对象
    return NET_ERR_SOCKET;
  }

  // 将sock对象记录到外部socket对象
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

  io->ret_len += 1;  // 模拟发送数据
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