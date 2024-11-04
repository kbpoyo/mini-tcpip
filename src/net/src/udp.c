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

/**
 * @brief 打印已分配的udp socket对象列表
 *
 */
static void udp_disp_list(void) {
  nlist_node_t *node = (nlist_node_t *)0;
  sock_t *sock = (sock_t *)0;
  int index = 0;

  plat_printf("---------------udp socket list:---------------\n");
  nlist_for_each(node, &udp_list) {
    sock = nlist_entry(node, sock_t, node);
    plat_printf("[%d]:", index++);
    netif_dum_ip(" local ip: ", &sock->local_ip);
    plat_printf(" local port: %d", sock->local_port);
    netif_dum_ip(" remote ip: ", &sock->remote_ip);
    plat_printf(" remote port: %d\n", sock->remote_port);
  }

  plat_printf("----------------------------------------------\n");
}

/**
 * @brief 打印udp数据包
 *
 * @param pkt
 */
static void udp_disp_pkt(udp_pkt_t *pkt) {
  plat_printf("---------------- udp packet ----------------\n");
  plat_printf("\tsrc port: %d\n", net_ntohs(pkt->udp_hdr.src_port));
  plat_printf("\tdest port: %d\n", net_ntohs(pkt->udp_hdr.dest_port));
  plat_printf("\ttotal len: %d\n", net_ntohs(pkt->udp_hdr.total_len));
  plat_printf("\tchecksum: 0x%04x\n", net_ntohs(pkt->udp_hdr.checksum));
  plat_printf("---------------------------------------------\n");
}

#else

#define udp_disp_list()
#define udp_disp_pkt(pkt)

#endif

/**
 * @brief 初始化udp模块资源
 *
 * @return net_err_t
 */
net_err_t udp_module_init(void) {
  dbg_info(DBG_UDP, "init udp module...");

  // 初始化udp socket对象内存块管理对象
  // 只由工作线程访问，不需要加锁
  mblock_init(&udp_mblock, udp_tbl, sizeof(udp_t), UDP_MAXCNT, NLOCKER_NONE);

  // 初始化udp socket对象挂载链表
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
  if (nlist_is_mount(&udp->sock_base.node)) {
    nlist_remove(&udp_list, &udp->sock_base.node);
  }

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
  static uint16_t last_alloc_port = NET_PORT_START - 1;
  for (int i = NET_PORT_START; i < NET_PORT_END; i++) {
    last_alloc_port = (last_alloc_port + 1 % NET_PORT_END);
    last_alloc_port = last_alloc_port ? last_alloc_port : NET_PORT_START;
    if (!udp_port_is_used(last_alloc_port)) {
      // 本地端口号未被使用，分配该端口号
      sock->local_port = last_alloc_port;
      udp_disp_list();
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
static net_err_t udp_send(ipaddr_t *dest_ip, uint16_t dest_port,
                          ipaddr_t *src_ip, uint16_t src_port, pktbuf_t *buf) {
  // 检查是否已指定源ip地址，若没有则查找路由表获取合适的源ip地址
  if (ipaddr_is_any(src_ip)) {
    route_entry_t *rt_entry = route_find(dest_ip);
    if (!rt_entry) {
      dbg_error(DBG_UDP, "route entry not found.");
      return NET_ERR_UDP;
    }
    src_ip = &rt_entry->netif->ipaddr;
  }

  // 为数据包添加udp头部,并设置udp头部信息
  net_err_t err =
      pktbuf_header_add(buf, sizeof(udp_hdr_t), PKTBUF_ADD_HEADER_CONT);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_UDP, "add header failed.");
    return err;
  }
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
  pktbuf_t *pktbuf = pktbuf_alloc(buf_len);  //!!! 获取数据包
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
 * @brief 从sock对象中接收数据，并记录发送方socket地址。
 *       且以数据包为单位进行读取，若未一次将数据包读取完，数据包也将被丢弃，
 *       ip协议限制ip数据包大小不超过64kb(包括ip头部和udp头部)，
 *       所以buf缓冲区最大为64kb，且每一个包也暗含了数据的边界信息。
 *       ! 上层应用必须保证buf缓冲区大小足够大，以接收一个完整的数据包。
 *       ! 若无法接收完整的数据包，将导致数据丢失，返回错误。
 *       ! 读取成功后返回的一定是该数据包的大小。
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
static net_err_t udp_recvfrom(struct _sock_t *sock, void *buf, size_t buf_len,
                              int flags, struct net_sockaddr *src,
                              net_socklen_t *src_len, ssize_t *ret_recv_len) {
  // 将基类sock对象转换为udp对象，并从其接收缓冲区链表中获取数据包
  udp_t *udp = (udp_t *)sock;
  nlist_node_t *node = nlist_remove_first(&udp->recv_buf_list);
  if (!node) {  // 接收缓冲区链表为空, 通知调用者等待
    return NET_ERR_NEEDWAIT;
  }
  pktbuf_t *pktbuf = nlist_entry(node, pktbuf_t, node);  //!!! 获取数据包

  // 获取并移除数据包已修改过的udp头部，并解析源ip地址和端口号到socket地址对象中
  if (src) {  // 需要解析源socket地址
    udp_remote_info_t *remote_info = pktbuf_data_ptr(pktbuf);
    struct net_sockaddr_in *src_addr = (struct net_sockaddr_in *)src;
    src_addr->sin_family = AF_INET;  // 只支持IPv4
    src_addr->sin_port = remote_info->port;
    src_addr->sin_addr.s_addr = *(uint32_t *)remote_info->ip;
  }
  pktbuf_header_remove(pktbuf, sizeof(udp_hdr_t));

  // 并将数据包的数据部分拷贝到接收缓冲区
  pktbuf_acc_reset(pktbuf);
  int data_len = pktbuf_total_size(pktbuf);
  int copy_len = data_len > buf_len ? buf_len : data_len;
  net_err_t err = pktbuf_read(pktbuf, buf, copy_len);
  pktbuf_free(pktbuf);  //!!! 释放数据包
  if (err != NET_ERR_OK) {
    dbg_error(DBG_UDP, "pktbuf read failed.");
    return err;
  }

  // 返回接收到的数据大小和源socket地址大小
  if (src_len) {
    *src_len = sizeof(struct net_sockaddr_in);
  }
  *ret_recv_len = copy_len;
  return NET_ERR_OK;
}

/**
 * @brief 释放一个原始socket对象
 *
 * @param sock
 * @return net_err_t
 */
static net_err_t udp_close(sock_t *sock) {
  // 将sock对象转换为udp对象
  udp_t *udp = (udp_t *)sock;

  // 释放sock对象的接收缓冲区链表
  nlist_node_t *node = (nlist_node_t *)0;
  while ((node = nlist_remove_first(&udp->recv_buf_list))) {
    pktbuf_t *pktbuf = nlist_entry(node, pktbuf_t, node);  //!!! 获取数据包
    pktbuf_free(pktbuf);                                   //!!! 释放数据包
  }

  // 销毁基类sock对象持有的资源，并释放原始sock对象
  sock_uninit(sock);
  udp_free(udp);

  // 显示udp对象列表
  udp_disp_list();

  return NET_ERR_OK;
}

/**
 * @brief udp socket连接目标socket地址
 *
 * @param sock
 * @param addr
 * @param addrlen
 * @return net_err_t
 */
static net_err_t udp_connect(sock_t *sock, const struct net_sockaddr *addr,
                             net_socklen_t addrlen) {
  net_err_t err = sock_connect(sock, addr, addrlen);
  udp_disp_list();
  return err;
}

/**
 * @brief udp socket绑定本地ip和端口号
 *
 * @param sock
 * @param addr
 * @param addrlen
 * @return net_err_t
 */
static net_err_t udp_bind(sock_t *sock, const struct net_sockaddr *addr,
                          net_socklen_t addrlen) {
  // 判断是否已绑定本地ip和端口号
  if (sock->local_port) {
    dbg_error(DBG_UDP, "socket has bind.");
    return NET_ERR_UDP;
  }

  // 获取端口号, 并判断端口号是否已被绑定
  const struct net_sockaddr_in *addr_in = (const struct net_sockaddr_in *)addr;
  ipaddr_t local_ip;
  ipaddr_from_bytes(&local_ip, addr_in->sin_addr.s_addr_bytes);
  uint16_t local_port = net_ntohs(addr_in->sin_port);
  nlist_node_t *node = (nlist_node_t *)0;
  nlist_for_each(node, &udp_list) {
    sock_t *sock = nlist_entry(node, sock_t, node);
    if (sock->local_port == local_port &&
        ipaddr_is_equal(&sock->local_ip, &local_ip)) {
      dbg_error(DBG_UDP, "port has bind.");
      return NET_ERR_UDP;
    }
  }

  // socket未进行绑定，且端口号未被使用，绑定本地ip和端口号
  net_err_t err = sock_bind(sock, &local_ip, local_port);
  udp_disp_list();

  return err;
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
      .sendto = udp_sendto,      // 独立实现接口
      .recvfrom = udp_recvfrom,  // 独立实现接口
      .setopt = sock_setopt,     // 继承基类的实现
      .close = udp_close,        // 独立实现接口
      .connect = udp_connect,    // 独立实现接口
      .bind = udp_bind,
      .send = sock_send,  // 继承基类的实现
      .recv = sock_recv,  // 继承基类的实现
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

/**
 * @brief 根据源ip地址、源端口号、目的ip地址、目的端口号查找udp
 *
 * local_ip | local_port | remote_ip | remote_port |
 * 0       | 指定        | 0         | 0           |
 * 可接收任意源ip地址和端口号的udp数据包(服务器模式) 指定    | 指定        | 0
 * | 0           | 从指定网络接口接任意源ip地址和端口号的udp数据包(限制接收网卡)
 * 0       | 0           | 指定      | 指定        |
 * 只与指定的目的ip地址和端口号之间收发udp数据包(客户端模式)
 * TODO: 细化匹配规则，若端口号重复，则采用匹配项数最多的规则
 *
 * @param src_ip
 * @param src_port
 * @param dest_ip
 * @param dest_port
 * @return udp_t*
 */
static udp_t *udp_find(ipaddr_t *src_ip, uint16_t src_port, ipaddr_t *dest_ip,
                       uint16_t dest_port) {
  nlist_node_t *node = (nlist_node_t *)0;
  sock_t *sock = (sock_t *)0;

  // TODO: 优化查找算法，选出四元组匹配项最多的udp sock对象
  nlist_for_each(node, &udp_list) {
    sock = nlist_entry(node, sock_t, node);
    if (sock->local_port != dest_port) {  // 目的端口号不匹配
      continue;
    }

    if (!ipaddr_is_any(&sock->local_ip) &&
        !ipaddr_is_equal(&sock->local_ip, dest_ip)) {
      // udp sock对象已绑定本地ip地址，但与指定的目的ip地址不匹配
      continue;
    }

    if (!ipaddr_is_any(&sock->remote_ip) &&
        !ipaddr_is_equal(&sock->remote_ip, src_ip)) {
      // udp sock对象已绑定远端ip地址，但与指定的源ip地址不匹配
      continue;
    }

    if (sock->remote_port && sock->remote_port != src_port) {
      // udp sock对象已绑定远端端口号，但与指定的源端口号不匹配
      continue;
    }

    // 找到匹配的udp sock对象
    return (udp_t *)sock;
  }

  return (udp_t *)0;
}

/**
 * @brief 检查udp数据包是否正确
 *
 * @param pkt
 * @param size
 * @param src_ip
 * @param dest_ip
 * @return net_err_t
 */
static net_err_t udp_check(pktbuf_t *udp_buf, ipaddr_t *src_ip,
                           ipaddr_t *dest_ip) {
  // 获取udp数据包对象和数据包大小
  udp_pkt_t *pkt = (udp_pkt_t *)pktbuf_data_ptr(udp_buf);
  int size = pktbuf_total_size(udp_buf);

  // 检查udp数据包大小是否正确
  if ((size < sizeof(udp_hdr_t)) ||
      (size < net_ntohs(pkt->udp_hdr.total_len))) {
    dbg_error(DBG_UDP, "udp packet size error.");
    return NET_ERR_UDP;
  }

  // 检查校验和是否正确
  if (pkt->udp_hdr.checksum) {  // 校验和不为0，需要进行校验
    uint16_t chksum = tools_checksum16_pseudo_head(udp_buf, dest_ip, src_ip,
                                                   NET_PROTOCOL_UDP);
    if (chksum != 0) {
      dbg_error(DBG_UDP, "udp checksum error.");
      return NET_ERR_UDP;
    }
  }

  return NET_ERR_OK;
}

/**
 * @brief udp协议层接收数据包
 *
 * @param udp_buf udp数据包
 * @param src_ip 发送方ip地址
 * @param dest_ip 接收方ip地址
 * @return net_err_t
 */
net_err_t udp_recv(pktbuf_t *buf, ipaddr_t *src_ip, ipaddr_t *dest_ip) {
  // 获取ip包头部大小，并设置ip头部和udp头部在内存上的连续性
  int ip_hdr_len = ipv4_get_hdr_size((ipv4_pkt_t *)pktbuf_data_ptr(buf));
  net_err_t err = pktbuf_set_cont(buf, ip_hdr_len + sizeof(udp_hdr_t));
  if (err != NET_ERR_OK) {
    dbg_error(DBG_UDP, "pktbuf set cont failed.");
    return err;
  }

  // 获取udp头部对象, 并提取端口号信息
  udp_hdr_t *udp_hdr = (udp_hdr_t *)(pktbuf_data_ptr(buf) + ip_hdr_len);
  uint16_t dest_port = net_ntohs(udp_hdr->dest_port);
  uint16_t src_port = net_ntohs(udp_hdr->src_port);

  // 通过端口信息查找绑定的udp socket对象
  udp_t *udp = udp_find(src_ip, src_port, dest_ip, dest_port);
  if (!udp) {
    dbg_warning(DBG_UDP, "udp socket not found.");
    // 未找到匹配的udp
    // socket对象，返回不可达错误，通知ip层发送icmp端口不可达报文
    // 返回不可达错误前不能修改数据包内容，需要尽可能多的将错误数据包的内容返回给发送方
    return NET_ERR_UNREACH;
  }

  // 移除ip头部并计算udp数据包校验和判断数据包是否正确
  pktbuf_header_remove(buf, ip_hdr_len);
  err = udp_check(buf, src_ip, dest_ip);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_UDP, "udp check failed.");
    return err;
  }

  // 修改udp头部用于记录远端ip地址和端口号
  udp_remote_info_t *remote_info = (udp_remote_info_t *)pktbuf_data_ptr(buf);
  *(uint32_t *)remote_info->ip =
      src_ip->addr;  // 端口号不需要修改，默认在前两个字节

  // 将数据包放入udp sock对象的接收缓存链表，等待应用层接收
  if (nlist_count(&udp->recv_buf_list) <
      UDP_RECV_MAXCNT) {  // 接收缓冲区链表未满, 缓存数据包
    nlist_insert_last(&udp->recv_buf_list,
                      &buf->node);  //!!! 数据包转交

    // 唤醒等待接收数据的线程
    sock_wakeup(&udp->sock_base, SOCK_WAIT_READ, NET_ERR_OK);
  } else {             // 接收缓冲区链表已满, 丢弃数据包
    pktbuf_free(buf);  //!!! 释放数据包
    dbg_warning(DBG_UDP, "recv buf list is full.");
  }

  return NET_ERR_OK;
}
