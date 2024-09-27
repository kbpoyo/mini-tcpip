/**
 * @file udp.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief udp传输协议模块
 * @version 0.1
 * @date 2024-09-26
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "udp.h"

#include "mblock.h"
#include "nlist.h"
#include "protocol.h"
#include "route.h"
#include "tools.h"

static udp_t udp_tbl[UDP_MAXCNT];  // udp socket对象表
static mblock_t udp_mblock;        // udp socket对象内存块管理对象
static nlist_t udp_list;           // 挂载已分配的udp socket对象链表

#if DBG_DISP_ENABLED(DBG_UDP)

static void udp_disp_list(void) {
  nlist_node_t *node = (nlist_node_t *)0;
  sock_t *sock = (sock_t *)0;
  int index = 0;

  plat_printf("---------------udp socket list:---------------\n");
  nlist_for_each(node, &udp_list) {
    sock = nlist_entry(node, sock_t, node);
    plat_printf("[%d]:", index++);
    netif_dum_ip(" local ip: ", &sock->local_ip);
    netif_dum_ip(" remote ip: ", &sock->remote_ip);
    plat_printf("\n");
  }
}

#else

#define udp_disp_list()

#endif

/**
 * @brief 初始化udp模块资源
 *
 * @return net_err_t
 */
net_err_t udp_module_init(void) {
  dbg_info(DBG_UDP, "init udp module...");

  // 初始化原始socket对象内存块管理对象
  // 只由工作线程访问，不需要加锁
  mblock_init(&udp_mblock, udp_tbl, sizeof(udp_t), UDP_MAXCNT, NLOCKER_NONE);

  // 初始化原始socket对象挂载链表
  nlist_init(&udp_list);

  dbg_info(DBG_UDP, "init udp module ok.");

  return NET_ERR_OK;
}

/**
 * @brief 分配一个udp socket对象
 *
 * @return udp_t*
 */
static udp_t *udp_alloc(void) {
  udp_t *udp = (udp_t *)0;

  // 从内存块管理对象中分配一个udp对象
  udp = (udp_t *)mblock_alloc(&udp_mblock, -1);

  return udp;
}

/**
 * @brief 释放一个udp socket对象
 *
 * @param udp
 * @return void*
 */
static void *udp_free(udp_t *udp) {
  // 将udp对象从挂载链表中移除
  nlist_remove(&udp_list, &udp->sock_base.node);

  // 将udp对象内存块释放
  mblock_free(&udp_mblock, udp);
}

/**
 * @brief 判断指定端口号是否已被使用
 *
 * @param port
 * @return int
 */
static int udp_port_is_used(uint16_t port) {
  nlist_node_t *node = (nlist_node_t *)0;
  sock_t *sock = (sock_t *)0;

  nlist_for_each(node, &udp_list) {
    sock = nlist_entry(node, sock_t, node);
    if (sock->local_port == port) {
      return 1;
    }
  }

  return 0;
}

/**
 * @brief 分配一个本地端口号
 *
 * @param sock
 * @return net_err_t
 */
static net_err_t udp_port_alloc(sock_t *sock) {
  static uint16_t last_alloc_port = UDP_PORT_START - 1;
  for (int i = UDP_PORT_START; i < UDP_PORT_END; i++) {
    last_alloc_port = (last_alloc_port + 1 % UDP_PORT_END);
    last_alloc_port = last_alloc_port ? last_alloc_port : UDP_PORT_START;
    if (!udp_port_is_used(last_alloc_port)) {
      // 本地端口号未被使用，分配该端口号
      sock->local_port = last_alloc_port;
      return NET_ERR_OK;
    }
  }

  // 本地端口号分配失败
  return NET_ERR_UDP;
}

/**
 * @brief 通过udp传输协议发送数据
 *
 * @param dest_ip
 * @param dest_port
 * @param src_ip
 * @param src_port
 * @param buf
 * @return net_err_t
 */
net_err_t udp_send(ipaddr_t *dest_ip, uint16_t dest_port, ipaddr_t *src_ip,
                   uint16_t src_port, pktbuf_t *buf) {
  // 检查是否已指定源ip地址，若没有则查找路由表获取合适的源ip地址
  if (ipaddr_is_any(src_ip)) {
    route_entry_t *rt_entry = route_find(dest_ip);
    if (!rt_entry) {
      dbg_error(DBG_UDP, "route entry not found.");
      return NET_ERR_UDP;
    }
    src_ip = &rt_entry->netif->ipaddr;
  }

  // 为数据包添加udp头部
  net_err_t err =
      pktbuf_header_add(buf, sizeof(udp_hdr_t), PKTBUF_ADD_HEADER_CONT);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_UDP, "add header failed.");
    return err;
  }

  // 获取udp头部对象, 并设置udp头部信息
  udp_hdr_t *udp_hdr = pktbuf_data_ptr(buf);
  udp_hdr->src_port = net_htons(src_port);
  udp_hdr->dest_port = net_htons(dest_port);
  udp_hdr->total_len = net_htons(pktbuf_total_size(buf));
  udp_hdr->checksum = 0;

  // 计算udp头部校验和(携带伪头部)
  udp_hdr->checksum =
      tools_checksum16_pseudo_head(buf, dest_ip, src_ip, NET_PROTOCOL_UDP);

  // 通过ipv4传输协议发送数据
  err = ipv4_send(NET_PROTOCOL_UDP, dest_ip, src_ip, buf);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_UDP, "ipv4 send failed.");
    return err;
  }

  return NET_ERR_OK;
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
static net_err_t udp_sendto(struct _sock_t *sock, const void *buf,
                            size_t buf_len, int flags,
                            const struct net_sockaddr *dest,
                            net_socklen_t dest_len, ssize_t *ret_send_len) {
  // 获取ipv4模式的socket地址, 并提取目的ip地址和端口号
  ipaddr_t remote_ip;
  int remote_port = 0;
  const struct net_sockaddr_in *remote_addr =
      (const struct net_sockaddr_in *)dest;
  ipaddr_from_bytes(&remote_ip, remote_addr->sin_addr.s_addr_bytes);
  remote_port = net_ntohs(remote_addr->sin_port);

  // 若本地socket基类对象已调用bind绑定了远端地址，则判断是否与dest指定的地址一致
  if (!ipaddr_is_any(&sock->remote_ip) &&
      !ipaddr_is_equal(&sock->remote_ip, &remote_ip)) {
    // 已绑定远端地址， 但与dest指定的地址不一致
    dbg_error(DBG_UDP, "socket has bind and remote address not match.");
    return NET_ERR_UDP;
  }
  if (sock->remote_port && sock->remote_port != remote_port) {
    // 已绑定远端端口， 但与dest指定的端口不一致
    dbg_error(DBG_UDP, "socket has bind and remote port not match.");
    return NET_ERR_UDP;
  }

  // 若未绑定本地端口，则分配一个本地端口
  if (!sock->local_port &&
      ((sock->err_code = udp_port_alloc(sock)) != NET_ERR_OK)) {
    dbg_error(DBG_UDP, "alloc local port failed.");
    return sock->err_code;
  }

  // TODO: 若分配的数据包缓冲区大小不足，将数据分多次发送
  // 为待发送数据分配一个数据包缓冲区
  pktbuf_t *pktbuf = pktbuf_alloc(buf_len);  //!!! 分配数据包
  if (!pktbuf) {
    dbg_error(DBG_UDP, "no memory for pktbuf.");
    return NET_ERR_UDP;
  }

  // 将待发送数据拷包到数据包缓冲区
  net_err_t err = pktbuf_write(pktbuf, buf, buf_len);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_UDP, "pktbuf write failed.");
    pktbuf_free(pktbuf);  //!!! 释放数据包
    return err;
  }

  // 通过udp传输协议发送数据
  err = udp_send(&remote_ip, remote_port, &sock->local_ip, sock->local_port,
                 pktbuf);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_UDP, "udp send failed.");
    pktbuf_free(pktbuf);  //!!! 释放数据包
    return err;
  }

  // 发送成功，返回发送数据大小
  *ret_send_len += buf_len;

  return NET_ERR_OK;
}

/**
 * @brief 内部接口，创建一个udp socket对象
 *
 * @param family  协议族
 * @param protocol 上层协议
 * @return sock_t*
 */
sock_t *udp_create(int family, int protocol) {
  static const sock_ops_t udp_ops = {
      .sendto = udp_sendto,  // 独立实现接口
      //   .recvfrom = udp_recvfrom,  // 独立实现接口
      .setopt = sock_setopt,  // 继承基类的实现
      //   .close = udp_close,        // 独立实现接口
  };

  // 分配一个原始socket对象
  udp_t *udp = udp_alloc();
  if (!udp) {  // 内存块不足分配失败
    dbg_error(DBG_UDP, "no memory for udp socket.");
    return (sock_t *)0;
  }
  // 初始化该udp对象的基类socket对象, 并挂载到udp对象链表
  sock_init(&udp->sock_base, family, protocol, &udp_ops);
  nlist_insert_last(&udp_list, &udp->sock_base.node);
  udp_disp_list();

  // 初始化udp的数据包接收缓存链表
  nlist_init(&udp->recv_buf_list);

  // 使用基类sock记录udp sock的wait对象, 并初始化
  udp->sock_base.recv_wait = &udp->recv_wait;
  if (sock_wait_init(udp->sock_base.recv_wait) != NET_ERR_OK) {
    dbg_error(DBG_UDP, "sock wait init failed.");
    // 销毁基类sock对象持有的资源，并释放udp sock对象
    sock_uninit(&udp->sock_base);
    udp_free(udp);
    return (sock_t *)0;
  }

  // sock_base为第一个成员，起始地址相同，可直接转换
  return (sock_t *)udp;
}
