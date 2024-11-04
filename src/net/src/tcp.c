/**
 * @file tcp.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief tcp传输协议模块
 * @version 0.1
 * @date 2024-10-10
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "tcp.h"

#include "mblock.h"
#include "protocol.h"
#include "route.h"
#include "tcp_send.h"
#include "tcp_state.h"

#define TCP_CREATE_WAIT 1    // 创建tcp socket时进行等待
#define TCP_CREATE_NOWAIT 0  // 创建tcp socket时不进行等待

static tcp_t tcp_tbl[TCP_MAXCNT];  // tcp socket对象表
static mblock_t tcp_mblock;        // tcp socket对象内存块管理对象
static nlist_t tcp_list;           // 挂载已分配的tcp socket对象链表

#if DBG_DISP_ENABLED(DBG_TCP)
void tcp_disp(char *msg, tcp_t *tcp) {
  plat_printf("msg: %s\n", msg);
  plat_printf("\tlocal port: %d, remote port: %d\n", tcp->sock_base.local_port,
              tcp->sock_base.remote_port);
  plat_printf("\tstate: %s\n", tcp_state_name(tcp->state));
}

void tcp_disp_pkt(char *msg, tcp_hdr_t *tcp_hdr, pktbuf_t *tcp_buf) {
  plat_printf("msg: %s\n", msg);
  plat_printf("\tsrc port: %u, dest port: %u\n", tcp_hdr->src_port,
              tcp_hdr->dest_port);
  plat_printf("\tseq: %u, ack: %u\n", tcp_hdr->seq, tcp_hdr->ack);
  plat_printf("\twin size: %d, urg ptr: %d\n", tcp_hdr->win_size,
              tcp_hdr->urg_ptr);
  plat_printf("\tdata len: %d\n",
              pktbuf_total_size(tcp_buf) - tcp_get_hdr_size(tcp_hdr));
  plat_printf("\tflag: ");
  if (tcp_hdr->f_fin) {
    plat_printf("FIN ");
  }
  if (tcp_hdr->f_syn) {
    plat_printf("SYN ");
  }
  if (tcp_hdr->f_rst) {
    plat_printf("RST ");
  }
  if (tcp_hdr->f_psh) {
    plat_printf("PSH ");
  }
  if (tcp_hdr->f_ack) {
    plat_printf("ACK ");
  }
  if (tcp_hdr->f_urg) {
    plat_printf("URG ");
  }
  if (tcp_hdr->f_ece) {
    plat_printf("ECE ");
  }
  if (tcp_hdr->f_cwr) {
    plat_printf("CWR ");
  }
  plat_printf("\n");
}

void tcp_disp_list(void) {
  plat_printf("---------------tcp socket list:---------------\n");
  nlist_node_t *node = (nlist_node_t *)0;
  sock_t *sock = (sock_t *)0;
  int index = 0;
  nlist_for_each(node, &tcp_list) {
    sock = nlist_entry(node, sock_t, node);
    plat_printf("[%d]:", index++);
    tcp_disp("", (tcp_t *)sock);
  }

  plat_printf("----------------------------------------------\n");
}

#endif

net_err_t tcp_module_init(void) {
  dbg_info(DBG_TCP, "init tcp module ......");

  // 初始化tcp socket对象内存块管理对象
  // 只由工作线程访问，不需要加锁
  mblock_init(&tcp_mblock, tcp_tbl, sizeof(tcp_t), TCP_MAXCNT, NLOCKER_NONE);

  // 初始化tcp socket对象挂载链表
  nlist_init(&tcp_list);

  dbg_info(DBG_TCP, "init tcp module ok.");
  return NET_ERR_OK;
}

/**
 * @brief 初始化一个tcp数据包信息结构
 *
 * @param tcp_info
 * @param tcp_buf
 * @param local_ip
 * @param remote_ip
 */
void tcp_info_init(tcp_info_t *tcp_info, pktbuf_t *tcp_buf, ipaddr_t *local_ip,
                   ipaddr_t *remote_ip) {
  // 记录tcp数据包和头部
  tcp_info->tcp_buf = tcp_buf;
  tcp_info->tcp_hdr = (tcp_hdr_t *)pktbuf_data_ptr(tcp_buf);

  // 记录本地ip(主机ip)和远端ip(发送方的ip)
  ipaddr_copy(&tcp_info->local_ip, local_ip);
  ipaddr_copy(&tcp_info->remote_ip, remote_ip);

  // 计算tcp数据包的有效数据长度
  tcp_info->data_len =
      pktbuf_total_size(tcp_buf) - tcp_get_hdr_size(tcp_info->tcp_hdr);

  // 记录tcp数据包的序列号和序列号长度
  tcp_info->seq = tcp_info->tcp_hdr->seq;

  /**
   * tcp协议为每一个有效数据字节都分配一个序列号
   * 并且也为SYN和FIN标志位各分配一个序列号
   * 拥有序列号的数据才是tcp确保可靠传输的数据
   * 序列号长度将用于接收方进行累积确认
   */
  tcp_info->seq_len =
      tcp_info->data_len + tcp_info->tcp_hdr->f_syn + tcp_info->tcp_hdr->f_fin;
}

/**
 * @brief 根据wait参数来判断是否以阻塞方式获取一个空闲的tcp对象
 *
 * @param wait
 * @return tcp_t*
 */
static tcp_t *tcp_get_free(int wait) {
  tcp_t *tcp = (tcp_t *)0;

  // 从内存块管理对象中分配一个tcp对象
  tcp = (tcp_t *)mblock_alloc(&tcp_mblock, wait == TCP_CREATE_WAIT ? 0 : -1);

  return tcp;
}

/**
 * @brief 释放一个tcp socket对象
 *
 * @param tcp
 * @return void*
 */
static void *tcp_free(tcp_t *tcp) {
  // 销毨tcp对象的基础sock对象所持有的资源(主要是wait对象)
  sock_uninit(&tcp->sock_base);

  // 将tcp对象从挂载链表中移除
  if (nlist_is_mount(&tcp->sock_base.node)) {
    nlist_remove(&tcp_list, &tcp->sock_base.node);
  }

  // 将tcp对象内存块释放
  mblock_free(&tcp_mblock, tcp);

  tcp_disp_list();
}

/**
 * @brief 检查tcp端口号是否已被使用
 *
 * @param port
 * @return int
 */
static int tcp_port_is_used(uint16_t port) {
  nlist_node_t *node = (nlist_node_t *)0;
  nlist_for_each(node, &tcp_list) {
    sock_t *sock = nlist_entry(node, sock_t, node);
    if (sock->local_port == port) {
      return 1;
    }
  }

  return 0;
}

/**
 * @brief 分配一个本地端口号
 *
 * @return uint16_t
 */
static net_err_t tcp_alloc_port(sock_t *sock) {
#ifdef NET_MOD_DBG
  // 调试模式下，随机分配端口号，避免多次启动调试时端口号冲突
  srand((unsigned)time(NULL));
  int last_alloc_port =
      NET_PORT_START + (rand() % (NET_PORT_END - NET_PORT_START));
#else
  static uint16_t last_alloc_port = NET_PORT_START - 1;
#endif

  for (int i = NET_PORT_START; i < NET_PORT_END; i++) {
    last_alloc_port = (last_alloc_port + 1 % NET_PORT_END);
    last_alloc_port = last_alloc_port ? last_alloc_port : NET_PORT_START;
    if (!tcp_port_is_used(last_alloc_port)) {
      // 本地端口号未被使用，分配该端口号
      sock->local_port = last_alloc_port;
      tcp_disp_list();
      return NET_ERR_OK;
    }
  }

  return NET_ERR_TCP;
}

/**
 * @brief 为发送窗口获取一个随机的初始序号
 * TODO: 此处实现简单，可以使用更复杂的算法
 *
 * @return uint32_t
 */
static uint32_t tcp_get_isn(void) {
  static uint32_t isn = 1024;
  return isn++;
}

/**
 * @brief 初始化tcp连接结构
 *
 * @param tcp
 * @return net_err_t
 */
static net_err_t tcp_init_connect(tcp_t *tcp) {
  // 获取本次连接的初始序号
  tcp->send.isn = tcp_get_isn();

  // 设置发送窗口的未确认位和待发送位
  tcp->send.una = tcp->send.isn;
  tcp->send.nxt = tcp->send.isn;

  // 设置接收到的初始序号位
  tcp->recv.isn = 0;

  // 设置接收窗口的待接收位
  tcp->recv.nxt = 0;

  return NET_ERR_OK;
}

/**
 * @brief 建立tcp连接
 *
 * @param sock
 * @param addr
 * @param addrlen
 * @return net_err_t
 */
net_err_t tcp_connect(sock_t *sock, const struct net_sockaddr *addr,
                      net_socklen_t addrlen) {
  // 检查tcp对象状态是否为CLOSED
  if (((tcp_t *)sock)->state != TCP_STATE_CLOSED) {
    dbg_error(DBG_TCP, "tcp state error.");
    return NET_ERR_TCP;
  }

  // 提取远端ip地址和端口号
  const struct net_sockaddr_in *addr_in = (const struct net_sockaddr_in *)addr;
  ipaddr_from_bytes(&sock->remote_ip, addr_in->sin_addr.s_addr_bytes);
  sock->remote_port = net_ntohs(addr_in->sin_port);

  // 若未绑定本地端口，则分配一个本地端口
  if (sock->local_port == NET_PORT_EMPTY) {
    if (tcp_alloc_port(sock) != NET_ERR_OK) {
      dbg_error(DBG_TCP, "alloc local port failed.");
      return NET_ERR_TCP;
    }
  }

  // 若未绑定本地ip地址，则查找路由表，获取本地ip地址
  if (ipaddr_is_any(&sock->local_ip)) {
    route_entry_t *rt_entry = route_find(&sock->remote_ip);
    if (!rt_entry) {  // 路由表查找失败, 返回不可达错误
      dbg_error(DBG_TCP, "route find failed.");
      return NET_ERR_UNREACH;
    }
    ipaddr_copy(&sock->local_ip, &rt_entry->netif->ipaddr);
  }

  // 初始化tcp对象连接状态，包括发送窗口和接收窗口的边界信息
  if (tcp_init_connect((tcp_t *)sock) != NET_ERR_OK) {
    dbg_error(DBG_TCP, "init connect failed.");
    return NET_ERR_TCP;
  }

  // 发送连接请求(SYN)
  if (tcp_send_syn((tcp_t *)sock) != NET_ERR_OK) {
    dbg_error(DBG_TCP, "send syn failed.");
    return NET_ERR_TCP;
  }

  // SYN已发送，切换到SYN_SENT状态
  tcp_state_set((tcp_t *)sock, TCP_STATE_SYN_SENT);

  return NET_ERR_NEEDWAIT;
}

/**
 * @brief 关闭tcp连接
 *
 * @param sock
 * @return net_err_t
 */
net_err_t tcp_close(sock_t *sock) {
  // 转为对应tcp对象
  tcp_t *tcp = (tcp_t *)sock;

  // 根据tcp对象的状态进行不同的处理
  switch (tcp->state) {
    case TCP_STATE_CLOSED: {  // tcp已关闭，释放tcp对象
      dbg_info(DBG_TCP, "tcp closed.");
      tcp_free(tcp);
    } break;

    case TCP_STATE_SYN_SENT:
    case TCP_STATE_SYN_RCVD: {  // tcp正在等待连接建立，放弃该连接并释放tcp对象
      tcp_abort_connect(tcp, NET_ERR_TCP_CLOSE);
      tcp_free(tcp);
    } break;

    case TCP_STATE_CLOSE_WAIT: {  // tcp处于等待关闭状态(对端已关闭)，发送关闭请求
                                  // 并切换到LAST_ACK状态
      if (tcp_send_fin(tcp) != NET_ERR_OK) {
        dbg_error(DBG_TCP, "send fin failed.");
        return NET_ERR_TCP;
      }
      tcp_state_set(tcp, TCP_STATE_LAST_ACK);
      return NET_ERR_NEEDWAIT;  // 通知调用者等待对端对FIN的确认
    } break;

    case TCP_STATE_ESTABLISHED: {  // tcp连接已建立，发送关闭请求
      if (tcp_send_fin(tcp) != NET_ERR_OK) {
        dbg_error(DBG_TCP, "send fin failed.");
        return NET_ERR_TCP;
      }
      tcp_state_set(tcp, TCP_STATE_FIN_WAIT_1); // 切换到FIN_WAIT_1状态
      return NET_ERR_NEEDWAIT;  // 通知调用者等待对端对FIN的确认
    } break;

    case TCP_STATE_TIME_WAIT: {  // TODO: 暂时处：释放tcp对象
      tcp_free(tcp);
    } break;
    default:
      break;
  }

  return NET_ERR_OK;
}

/**
 * @brief 分配一个tcp socket对象
 *
 * @return tcp_t*
 */
static tcp_t *tcp_alloc(int family, int protocol, int wait) {
  static const sock_ops_t tcp_ops = {
      .connect = tcp_connect,  // 独立实现接口
      .close = tcp_close,      // 独立实现接口
      //   .sendto = tcp_sendto,      // 独立实现接口
      //   .recvfrom = tcp_recvfrom,  // 独立实现接口
      //   .setopt = sock_setopt,     // 继承基类的实现
      //   .bind = tcp_bind,
      //   .send = sock_send,  // 继承基类的实现
      //   .recv = sock_recv,  // 继承基类的实现
  };

  // 获取一个空闲的tcp对象
  tcp_t *tcp = tcp_get_free(wait);
  if (!tcp) {  // 内存块不足分配失败
    dbg_error(DBG_TCP, "no free tcp obj.");
    return (tcp_t *)0;
  }

  // 初始化tcp状态
  tcp->state = TCP_STATE_CLOSED;

  // 初始化tcp对象的基础socket对象
  plat_memset(tcp, 0, sizeof(tcp_t));
  net_err_t err = sock_init(&tcp->sock_base, family, protocol, &tcp_ops);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_TCP, "sock init failed.");
    goto tcp_alloc_failed;
  }

  // 初始化tcp对象的连接wait对象, 并用基类sock记录该wait对象
  if (sock_wait_init(&tcp->conn.wait) != NET_ERR_OK) {
    dbg_error(DBG_TCP, "conn wait init failed.");
    goto tcp_alloc_failed;
  }
  tcp->sock_base.conn_wait = &tcp->conn.wait;

  // 初始化tcp对象的发送wait对象, 并用基类sock记录该wait对象
  if (sock_wait_init(&tcp->send.wait) != NET_ERR_OK) {
    dbg_error(DBG_TCP, "send wait init failed.");
    goto tcp_alloc_failed;
  }
  tcp->sock_base.send_wait = &tcp->send.wait;

  // 初始化tcp对象的接收wait对象, 并用基类sock记录该wait对象
  if (sock_wait_init(&tcp->recv.wait) != NET_ERR_OK) {
    dbg_error(DBG_TCP, "recv wait init failed.");
    goto tcp_alloc_failed;
  }
  tcp->sock_base.recv_wait = &tcp->recv.wait;

  return tcp;

tcp_alloc_failed:
  tcp_free(tcp);
  return (tcp_t *)0;
}

/**
 * @brief 将一个tcp socket对象挂载到tcp对象链表中
 *
 * @param tcp
 */
static void tcp_insert(tcp_t *tcp) {
  nlist_insert_last(&tcp_list, &tcp->sock_base.node);

  dbg_assert(nlist_count(&tcp_list) <= TCP_MAXCNT, "tcp socket count error.");
}

/**
 * @brief 内部接口，创建一个tcp socket对象
 *
 * @param family  协议族
 * @param protocol 上层协议
 * @return sock_t*
 */
sock_t *tcp_create(int family, int protocol) {
  // 分配一个原始socket对象
  tcp_t *tcp = tcp_alloc(family, protocol, TCP_CREATE_WAIT);
  if (!tcp) {  // 内存块不足分配失败
    dbg_error(DBG_TCP, "no memory for tcp socket.");
    return (sock_t *)0;
  }

  tcp_insert(tcp);

  //   // 初始化该tcp对象的基类socket对象, 并挂载到tcp对象链表
  //   sock_init(&tcp->sock_base, family, protocol, &tcp_ops);
  //   nlist_insert_last(&tcp_list, &tcp->sock_base.node);
  //   //   tcp_disp_list();

  //   // 初始化tcp的数据包接收缓存链表
  //   //   nlist_init(&tcp->recv_buf_list);

  //   // 使用基类sock记录tcp sock的wait对象, 并初始化
  //   tcp->sock_base.recv_wait = &tcp->recv_wait;
  //   if (sock_wait_init(tcp->sock_base.recv_wait) != NET_ERR_OK) {
  //     dbg_error(DBG_tcp, "sock wait init failed.");
  //     // 销毁基类sock对象持有的资源，并释放tcp sock对象
  //     sock_uninit(&tcp->sock_base);
  //     tcp_free(tcp);
  //     return (sock_t *)0;
  //   }

  // sock_base为第一个成员，起始地址相同，可直接转换
  return (sock_t *)tcp;
}

/**
 * @brief 根据tcp信息结构查找对应的tcp对象
 *
 * @param info
 * @return tcp_t*
 */
tcp_t *tcp_find(tcp_info_t *info) {
  nlist_node_t *node = (nlist_node_t *)0;
  nlist_for_each(node, &tcp_list) {
    sock_t *tcp_sock = nlist_entry(node, sock_t, node);
    // 若tcp对象绑定了本地ip地址，则比较本地ip地址
    if (!ipaddr_is_any(&tcp_sock->local_ip) &&
        !ipaddr_is_equal(&tcp_sock->local_ip, &info->local_ip)) {
      continue;
    }

    // 比较四元组其余信息
    if (tcp_sock->local_port == info->tcp_hdr->dest_port &&
        tcp_sock->remote_port == info->tcp_hdr->src_port &&
        ipaddr_is_equal(&tcp_sock->remote_ip, &info->remote_ip)) {
      return (tcp_t *)tcp_sock;
    }
  }

  return (tcp_t *)0;
}

/**
 * @brief 舍弃当前tcp连接，唤醒等待在该tcp对象上的所有任务,
 *        并将tcp对象状态设置为CLOSED以等待本地持有者调用close关闭并释放该对象
 *       
 *
 * @param tcp
 * @param err
 * @return net_err_t
 */
net_err_t tcp_abort_connect(tcp_t *tcp, net_err_t err) {
  // 设置tcp状态为CLOSED
  tcp_state_set(tcp, TCP_STATE_CLOSED);

  // 唤醒等待在该tcp对象上的所有任务
  sock_wakeup(&tcp->sock_base, SOCK_WAIT_ALL, err);

  return NET_ERR_OK;
}