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

#include "mblock.h"
#include "net_cfg.h"

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
 * @brief 创建一个原始socket对象 
 * 
 * @param family  协议族
 * @param protocol 上层协议
 * @return sock_t* 
 */
sock_t *sockraw_create(int family, int protocol) {
    static const sock_ops_t sockraw_ops;

    // 分配一个原始socket对象
    sockraw_t * sockraw = sockraw_alloc();
    if (!sockraw) { // 内存块不足分配失败
        dbg_error(DBG_SOCKRAW, "no memory for raw socket.");
        return NET_ERR_SOCKRAW;
    }

    // 初始化该原始socket内部的基础socket对象
    net_err_t err = sock_init(&sockraw->sock_base, family, protocol, &sockraw_ops);
    if (err != NET_ERR_OK) {
        dbg_error(DBG_SOCKRAW, "sock init failed.");
        return NET_ERR_SOCKRAW;
    }

    // 将初始化后的sockraw对象挂载到记录链表中
    nlist_insert_last(&sockraw_list, &sockraw->sock_base.node);

    return (sock_t *)sockraw; //sock_base为一个成员，起始地址相同，可直接转换


}