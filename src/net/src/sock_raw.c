/**
 * @file sock_raw.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 协议栈内部的原始socket相关数据结构及方法
 * @version 0.1
 * @date 2024-08-01
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "sock_raw.h"

#include "ipv4.h"
#include "mblock.h"
#include "net_cfg.h"
#include "pktbuf.h"
#include "socket.h"

static sockraw_t sockraw_tbl[SOCKRAW_MAX_CNT];  // 原始socket对象表
static mblock_t sockraw_mblock;  // 原始socket对象内存块管理对象
static nlist_t sockraw_list;     // 挂载已分配的原始socket对象链表

/**
 * @brief 初始化原始socket模块资源
 *
 * @return net_err_t
 */
net_err_t sockraw_module_init(void) {
  dbg_info(DBG_SOCKRAW, "init sockraw module...");

  // 初始化原始socket对象内存块管理对象
  // 只由工作线程访问，不需要加锁
  mblock_init(&sockraw_mblock, sockraw_tbl, sizeof(sockraw_t), SOCKRAW_MAX_CNT,
              NLOCKER_NONE);

  // 初始化原始socket对象挂载链表
  nlist_init(&sockraw_list);

  dbg_info(DBG_SOCKRAW, "init sockraw module ok.");

  return NET_ERR_OK;
}

/**
 * @brief 分配一个原始socket对象
 *
 * @return sockraw_t*
 */
static sockraw_t *sockraw_alloc(void) {
  sockraw_t *sockraw = (sockraw_t *)0;

  // 从内存块管理对象中分配一个sockraw对象
  sockraw = (sockraw_t *)mblock_alloc(&sockraw_mblock, -1);

  return sockraw;
}

/**
 * @brief 内部接口，向目的socket地址发送数据
 *
 * @param sock 基类socket对象
 * @param buf 待发送数据缓冲区
 * @param buf_len 缓冲区大小
 * @param flags 选项设置标志位
 * @param dest 目的socket地址
 * @param dest_len socket地址对象大小
 * @param ret_send_len 实际发送数据大小
 * @return net_err_t
 */
net_err_t sockraw_sendto(struct _sock_t *sock, const void *buf, size_t buf_len,
                         int flags, const struct net_sockaddr *dest,
                         net_socklen_t dest_len, ssize_t *ret_send_len) {
  // 获取ipv4模式的socket地址
  const struct net_sockaddr_in *remote_addr =
      (const struct net_sockaddr_in *)dest;
  // 获取目的IP地址
  ipaddr_t remote_ip;
  ipaddr_from_bytes(&remote_ip, remote_addr->sin_addr.s_addr_bytes);

  // 若本地socket基类对象已调用bind绑定了远端地址，则判断是否与dest指定的地址一致
  if (!ipaddr_is_any(&sock->remote_ip) &&
      !ipaddr_is_equal(&sock->remote_ip, &remote_ip)) {  // 已绑定远端地址
    dbg_error(DBG_SOCKRAW, "socket has bind and remote address not match.");
    return NET_ERR_SOCKRAW;
  }

  // 为待发送数据分配一个数据包缓冲区
  pktbuf_t *pktbuf = pktbuf_alloc(buf_len);  //!!! 分配数据包
  if (!pktbuf) {
    dbg_error(DBG_SOCKRAW, "no memory for pktbuf.");
    return NET_ERR_SOCKRAW;
  }

  // 将待发送数据拷贝到数据包缓冲区
  net_err_t err = pktbuf_write(pktbuf, buf, buf_len);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_SOCKRAW, "pktbuf write failed.");
    pktbuf_free(pktbuf);  //!!! 释放数据包
    return err;
  }

  // 将数据包交给ipv4协议层发送
  err = ipv4_send(sock->protocol, &remote_ip, &(netif_get_default())->ipaddr,
                  pktbuf);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_SOCKRAW, "ipv4 send failed.");
    pktbuf_free(pktbuf);  //!!! 释放数据包
    return err;
  }

  // 发送成功，返回发送数据大小
  *ret_send_len += buf_len;

  return NET_ERR_OK;
}

/**
 * @brief 内部接口，从sock对象中接收数据，并记录发送方socket地址
 *
 * @param sock 基类socket对象
 * @param buf 接收数据缓冲区
 * @param buf_len 缓冲区大小
 * @param flags 选项设置标志位
 * @param src 源socket地址
 * @param src_len socket地址对象大小
 * @param ret_recv_len 实际接收数据大小
 * @return net_err_t
 */
net_err_t sockraw_recvfrom(struct _sock_t *sock, void *buf, size_t buf_len,
                           int flags, struct net_sockaddr *src,
                           net_socklen_t *src_len, ssize_t *ret_recv_len) {
  *src_len = 16;
  *ret_recv_len += buf_len;
  return NET_ERR_NEEDWAIT;
}

/**
 * @brief 创建一个原始socket对象
 *
 * @param family  协议族
 * @param protocol 上层协议
 * @return sock_t*
 */
sock_t *sockraw_create(int family, int protocol) {
  static const sock_ops_t sockraw_ops = {
      .sendto = sockraw_sendto,
      .recvfrom = sockraw_recvfrom,
  };

  // 分配一个原始socket对象
  sockraw_t *sockraw = sockraw_alloc();
  if (!sockraw) {  // 内存块不足分配失败
    dbg_error(DBG_SOCKRAW, "no memory for raw socket.");
    return (sock_t *)0;
  }

  // 初始化该原始socket内部的基础socket对象
  net_err_t err =
      sock_init(&sockraw->sock_base, family, protocol, &sockraw_ops);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_SOCKRAW, "sock init failed.");
    return (sock_t *)0;
  }

  // 将初始化后的sockraw对象挂载到记录链表中
  nlist_insert_last(&sockraw_list, &sockraw->sock_base.node);

  return (sock_t *)sockraw;  // sock_base为一个成员，起始地址相同，可直接转换
}
