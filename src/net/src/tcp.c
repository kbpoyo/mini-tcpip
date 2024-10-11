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

#define TCP_CREATE_WAIT 1    // 创建tcp socket时进行等待
#define TCP_CREATE_NOWAIT 0  // 创建tcp socket时不进行等待

static tcp_t tcp_tbl[TCP_MAXCNT];  // tcp socket对象表
static mblock_t tcp_mblock;        // tcp socket对象内存块管理对象
static nlist_t tcp_list;           // 挂载已分配的tcp socket对象链表

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
 * @brief 分配一个tcp socket对象
 *
 * @return tcp_t*
 */
static tcp_t *tcp_alloc(int family, int protocol, int wait) {
  static const sock_ops_t tcp_ops = {
      //   .sendto = tcp_sendto,      // 独立实现接口
      //   .recvfrom = tcp_recvfrom,  // 独立实现接口
      //   .setopt = sock_setopt,     // 继承基类的实现
      //   .close = tcp_close,        // 独立实现接口
      //   .connect = tcp_connect,    // 独立实现接口
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

  // 初始化tcp对象的基础socket对象
  plat_memset(tcp, 0, sizeof(tcp_t));
  net_err_t err = sock_init(&tcp->sock_base, family, protocol, &tcp_ops);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_TCP, "sock init failed.");
    mblock_free(&tcp_mblock, tcp);
    return (tcp_t *)0;
  }

  return tcp;
}

/**
 * @brief 释放一个tcp socket对象
 *
 * @param tcp
 * @return void*
 */
static void *tcp_free(tcp_t *tcp) {
  // 将tcp对象从挂载链表中移除
  nlist_remove(&tcp_list, &tcp->sock_base.node);

  // 将tcp对象内存块释放
  mblock_free(&tcp_mblock, tcp);
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