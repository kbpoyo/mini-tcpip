/**
 * @file pktbuf.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 数据包模块
 * @version 0.1
 * @date 2024-05-13
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "pktbuf.h"
#include "dbg.h"
#include "mblock.h"

static pktblk_t pktblk_pool[PKTBUF_BLK_CNT]; // 数据块池
static mblock_t pktblk_list; // 数据块链表(管理数据块池)

static pktbuf_t pktbuf_pool[PKTBUF_BUF_CNT];   // 数据包池
static mblock_t pktbuf_list;   // 数据包链表(管理数据包池)

static nlocker_t pkt_locker; // 数据包模块锁

/**
 * @brief 初始化数据包模块
 * 
 * @return net_err_t 
 */
net_err_t pktbuf_init(void) {
    dbg_info(DBG_PKTBUF, "pktbuf init....");

    nlocker_init(&pkt_locker, PKTBUF_LOCKER_TYPE);
    mblock_init(&pktblk_list, pktblk_pool, sizeof(pktblk_t), PKTBUF_BLK_CNT, PKTBUF_LOCKER_TYPE);
    mblock_init(&pktbuf_list, pktbuf_pool, sizeof(pktbuf_t), PKTBUF_BUF_CNT, PKTBUF_LOCKER_TYPE);



    dbg_info(DBG_PKTBUF, "pktbuf init success....");
    return NET_ERR_OK;
}