/**
 * @file pktbuf.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 数据包模块
 * @version 0.1
 * @date 2024-05-13
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef PKTBUF_H
#define PKTBUF_H

#include <stdint.h>
#include "nlist.h"
#include "net_cfg.h"
#include "net_err.h"

#define PKTBUF_LIST_INSERT_HEAD 0
#define PKTBUF_LIST_INSERT_TAIL 1

/**
 * @brief 定义数据块结构
 * 
 */
typedef struct _pktblk_t {
    nlist_node_t node;  // 数据块链表结点
    int data_size;  // 数据大小
    uint8_t *data;  // 数据起始地址
    uint8_t payload[PKTBUF_BLK_SIZE];    // 数据块的有效载荷大小

} pktblk_t;

/**
 * @brief 定义数据包结构
 * 
 */
typedef struct _pktbuf_t {
    int total_size; // 数据包总大小
    nlist_t blk_list;   // 数据块链表
    nlist_node_t node;  // 数据包链表结点
} pktbuf_t;



net_err_t pktbuf_init(void);

pktbuf_t *pktbuf_alloc(int size);
void pktbuf_free(pktbuf_t *pktbuf);

/**
 * 获取当前block的下一个子包
 */
static inline pktblk_t * pktbuf_blk_next(pktblk_t *blk) {
    nlist_node_t * next = nlist_node_next(&blk->node);
    return nlist_entry(next, pktblk_t, node);
}

/**
 * 取buf的第一个数据块
 * @param buf 数据缓存buf
 * @return 第一个数据块
 */
static inline pktblk_t * pktbuf_blk_first(pktbuf_t * buf) {
    nlist_node_t * first = nlist_first(&buf->blk_list);
    return nlist_entry(first, pktblk_t, node);
}

/**
 * 取buf的最后一个数据块
 * @param buf buf 数据缓存buf
 * @return 最后一个数据块
 */
static inline pktblk_t * pktbuf_blk_last(pktbuf_t * buf) {
    nlist_node_t * first = nlist_last(&buf->blk_list);
    return nlist_entry(first, pktblk_t, node);
}

static int inline pktbuf_total_size (pktbuf_t * buf) {
    return buf->total_size;
}

#endif  // PKTBUF_H