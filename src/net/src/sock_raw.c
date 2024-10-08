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
#include "tools.h"

static sockraw_t sockraw_tbl[SOCKRAW_MAXCNT];  // 原始socket对象表
static mblock_t sockraw_mblock;  // 原始socket对象内存块管理对象
static nlist_t sockraw_list;     // 挂载已分配的原始socket对象链表

#if DBG_DISP_ENABLED(DBG_SOCKRAW)

static void sockraw_disp_list(void) {
  nlist_node_t *node = (nlist_node_t *)0;
  sock_t *sock = (sock_t *)0;
  int index = 0;

  plat_printf("---------------raw socket list:---------------\n");
  nlist_for_each(node, &sockraw_list) {
    sock = nlist_entry(node, sock_t, node);
    plat_printf("[%d]:", index++);
    netif_dum_ip(" local ip: ", &sock->local_ip);
    netif_dum_ip(" remote ip: ", &sock->remote_ip);
    plat_printf("\n");
  }
}

#else

#define sockraw_disp_list()

#endif

/**
 * @brief 初始化原始socket模块资源
 *
 * @return net_err_t
 */
net_err_t sockraw_module_init(void) {
  dbg_info(DBG_SOCKRAW, "init sockraw module...");

  // 初始化原始socket对象内存块管理对象
  // 只由工作线程访问，不需要加锁
  mblock_init(&sockraw_mblock, sockraw_tbl, sizeof(sockraw_t), SOCKRAW_MAXCNT,
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
 * @brief 释放一个原始socket对象
 *
 * @param sockraw
 * @return void*
 */
static void *sockraw_free(sockraw_t *sockraw) {
  // 将sockraw对象从挂载链表中移除
  nlist_remove(&sockraw_list, &sockraw->sock_base.node);

  // 将sockraw对象内存块释放
  mblock_free(&sockraw_mblock, sockraw);
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
static net_err_t sockraw_sendto(struct _sock_t *sock, const void *buf,
                                size_t buf_len, int flags,
                                const struct net_sockaddr *dest,
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

  // TODO: 若分配的数据包缓冲区大小不足，将数据分多次发送
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
  err = ipv4_send(sock->protocol, &remote_ip, &sock->local_ip, pktbuf);
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
static net_err_t sockraw_recvfrom(struct _sock_t *sock, void *buf,
                                  size_t buf_len, int flags,
                                  struct net_sockaddr *src,
                                  net_socklen_t *src_len,
                                  ssize_t *ret_recv_len) {
  // 将基类sock对象转换为sockraw对象
  sockraw_t *sockraw = (sockraw_t *)sock;

  // 从sock对象的接收缓冲区链表中获取一个数据包
  nlist_node_t *node = nlist_remove_first(&sockraw->recv_buf_list);
  if (!node) {  // 接收缓冲区链表为空, 通知调用者等待
    return NET_ERR_NEEDWAIT;
  }
  pktbuf_t *pktbuf = nlist_entry(node, pktbuf_t, node);  //!!! 获取数据包

  // 获取数据包的ipv4头部, 并解析源ip地址信息到socket地址对象中
  ipv4_hdr_t *ipv4_hdr = (ipv4_hdr_t *)pktbuf_data_ptr(pktbuf);
  struct net_sockaddr_in *src_addr = (struct net_sockaddr_in *)src;
  src_addr->sin_family = AF_INET;  // 只支持IPv4
  src_addr->sin_port = 0;
  src_addr->sin_addr.s_addr = *(uint32_t *)ipv4_hdr->src_ip;

  // 将数据包的数据部分拷贝到接收缓冲区
  pktbuf_acc_reset(pktbuf);
  int data_len = pktbuf_total_size(pktbuf);
  int copy_len = data_len > buf_len ? buf_len : data_len;
  net_err_t err = pktbuf_read(pktbuf, buf, copy_len);
  pktbuf_free(pktbuf);  //!!! 释放数据包
  if (err != NET_ERR_OK) {
    dbg_error(DBG_SOCKRAW, "pktbuf read failed.");
    return err;
  }

  // 返回接收到的数据大小和源socket地址大小
  *src_len = sizeof(struct net_sockaddr_in);
  *ret_recv_len = copy_len;
  return NET_ERR_OK;
}

/**
 * @brief 释放一个原始socket对象
 *
 * @param sock
 * @return net_err_t
 */
static net_err_t sockraw_close(sock_t *sock) {
  // 将sock对象转换为sockraw对象
  sockraw_t *sockraw = (sockraw_t *)sock;

  // 释放sock对象的接收缓冲区链表
  nlist_node_t *node = (nlist_node_t *)0;
  while ((node = nlist_remove_first(&sockraw->recv_buf_list))) {
    pktbuf_t *pktbuf = nlist_entry(node, pktbuf_t, node);  //!!! 获取数据包
    pktbuf_free(pktbuf);                                   //!!! 释放数据包
  }

  // 销毁基类sock对象持有的资源，并释放原始sock对象
  sock_uninit(sock);
  sockraw_free(sockraw);

  // 显示sockraw对象列表
  sockraw_disp_list();

  return NET_ERR_OK;
}

/**
 * @brief raw socket连接目标socket地址
 *
 * @param sock
 * @param addr
 * @param addrlen
 * @return net_err_t
 */
static net_err_t sockraw_connect(sock_t *sock, const struct net_sockaddr *addr,
                                 net_socklen_t addrlen) {
  net_err_t err = sock_connect(sock, addr, addrlen);
  sockraw_disp_list();
  return err;
}

/**
 * @brief raw socket绑定socket地址
 *
 * @param sock
 * @param addr
 * @param addrlen
 * @return net_err_t
 */
static net_err_t sockraw_bind(sock_t *sock, const struct net_sockaddr *addr,
                              net_socklen_t addrlen) {
  // 获取ip地址和端口号
  const struct net_sockaddr_in *addr_in = (const struct net_sockaddr_in *)addr;
  ipaddr_t local_ip;
  ipaddr_from_bytes(&local_ip, addr_in->sin_addr.s_addr_bytes);
  int local_port = net_ntohs(addr_in->sin_port);


  // 绑定本地ip和端口号(在raw socket中不使用端口号，所以端口号一般为0)
  net_err_t err = sock_bind(sock, &local_ip, local_port);
  sockraw_disp_list();

  return NET_ERR_OK;
}

/**
 * @brief 内部接口，创建一个原始socket对象
 *
 * @param family  协议族
 * @param protocol 上层协议
 * @return sock_t*
 */
sock_t *sockraw_create(int family, int protocol) {
  static const sock_ops_t sockraw_ops = {
      .sendto = sockraw_sendto,      // 独立实现接口
      .recvfrom = sockraw_recvfrom,  // 独立实现接口
      .setopt = sock_setopt,         // 继承基类的实现
      .close = sockraw_close,        // 独立实现接口
      .send = sock_send,             // 继承基类的实现
      .recv = sock_recv,             // 继承基类的实现
      .connect = sockraw_connect,    // 独立实现接口
      .bind = sockraw_bind,          // 继承基类的实现
  };

  // 分配一个原始socket对象
  sockraw_t *sockraw = sockraw_alloc();
  if (!sockraw) {  // 内存块不足分配失败
    dbg_error(DBG_SOCKRAW, "no memory for raw socket.");
    return (sock_t *)0;
  }
  // 初始化该sockraw对象的基类socket对象
  sock_init(&sockraw->sock_base, family, protocol, &sockraw_ops);
  // 将初始化成功的基类sock对象挂载到sockraw的记录链表中
  nlist_insert_last(&sockraw_list, &sockraw->sock_base.node);
  // 显示sockraw对象列表
  sockraw_disp_list();

  // 使用基类sock记录raw sock的wait对象, 并初始化
  sockraw->sock_base.recv_wait = &sockraw->recv_wait;
  if (sock_wait_init(sockraw->sock_base.recv_wait) != NET_ERR_OK) {
    dbg_error(DBG_SOCKRAW, "sock wait init failed.");
    goto create_failed;
  }

  // 初始化sockraw的数据包接收缓存链表
  nlist_init(&sockraw->recv_buf_list);

  return (sock_t *)sockraw;  // sock_base为第一个成员，起始地址相同，可直接转换

create_failed:
  sock_uninit(&sockraw->sock_base);
  sockraw_free(sockraw);
  return (sock_t *)0;
}

/**
 * @brief 根据目的ip地址、源ip地址和上层协议类型，
 * 在sockraw_list(当前系统正在使用的sockraw对象的记录链表)中查找对应的原始socket对象。
 *
 * @param dest_ip
 * @param src_ip
 * @param protocol
 * @return sockraw_t*
 */
static sockraw_t *sockraw_find(ipaddr_t *dest_ip, ipaddr_t *src_ip,
                               int protocol) {
  sockraw_t *sockraw = (sockraw_t *)0;
  nlist_node_t *node = (nlist_node_t *)0;

  nlist_for_each(node, &sockraw_list) {
    // 获取sockraw对象
    sockraw = nlist_entry(node, sockraw_t, sock_base.node);

    // 检查协议类型是否一致
    if (sockraw->sock_base.protocol &&
        sockraw->sock_base.protocol != protocol) {
      continue;
    }

    // 检查目的ip地址是否为sock记录的本地ip地址(ip地址若为空则默认匹配)
    if (!ipaddr_is_any(&sockraw->sock_base.local_ip) &&
        !ipaddr_is_equal(&sockraw->sock_base.local_ip, dest_ip)) {
      continue;
    }

    // 检查源ip地址是否为sock记录的远端ip地址
    if (!ipaddr_is_any(&sockraw->sock_base.remote_ip) &&
        !ipaddr_is_equal(&sockraw->sock_base.remote_ip, src_ip)) {
      continue;
    }

    // 返回找到的sockraw对象
    return sockraw;
  }

  return (sockraw_t *)0;
}

/**
 * @brief
 * 由工作线程调用，从网络层接收一个原始的ip数据包，
 * 可以处理的数据包类型有：IP, ICMP
 *
 * @param raw_buf
 * @return net_err_t
 */
net_err_t sockraw_recv_pktbuf(pktbuf_t *raw_ip_buf) {
  // 获取ipv4数据包的头部
  ipv4_hdr_t *ipv4_hdr = (ipv4_hdr_t *)pktbuf_data_ptr(raw_ip_buf);

  // 获取目的ip地址和源ip地址
  ipaddr_t dest_ip, src_ip;
  ipaddr_from_bytes(&dest_ip, ipv4_hdr->dest_ip);
  ipaddr_from_bytes(&src_ip, ipv4_hdr->src_ip);

  // TODO: 后续会将数据包交给所有匹配的raw socket对象，而不是只交给一个
  //  根据通信两端的ip地址信息和上层协议信息，查找对应的原始socket对象
  sockraw_t *sockraw = sockraw_find(&dest_ip, &src_ip, ipv4_hdr->tran_proto);
  if (!sockraw) {
    dbg_error(DBG_SOCKRAW, "no raw socket found.");
    return NET_ERR_SOCKRAW;
  }

  // 将接收到的数据包缓冲区添加到sockraw对象的接收缓冲区链表
  if (nlist_count(&sockraw->recv_buf_list) <
      SOCKRAW_RECV_MAXCNT) {  // 接收缓冲区链表未满
    nlist_insert_last(&sockraw->recv_buf_list,
                      &raw_ip_buf->node);  //!!! 数据包转交

    // 已缓存数据包，唤醒等待接收数据的线程
    sock_wakeup(&sockraw->sock_base, SOCK_WAIT_READ, NET_ERR_OK);
  } else {                    // 接收缓冲区链表已满, 丢弃数据包
    pktbuf_free(raw_ip_buf);  //!!! 释放数据包
    dbg_warning(DBG_SOCKRAW, "recv buf list is full.");
  }

  return NET_ERR_OK;
}