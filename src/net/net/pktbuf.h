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

#define PKTBUF_LIST_INSERT_HEAD 0   // 使用头插法
#define PKTBUF_LIST_INSERT_TAIL 1   // 使用尾插法
#define PKTBUF_ADD_HEADER_UNCONT 0    // 增添数据块时是否连续
#define PKTBUF_ADD_HEADER_CONT 1    // 增添数据块时是否连续


/**
 * @brief 定义数据块结构
 * 
 */
typedef struct _pktblk_t {
    nlist_node_t node;  // 数据块链表结点, 放在第一个元素的位置，方便调试观察在哪个列表中
    int data_size;  // 数据大小
    uint8_t *data;  // 数据起始地址
    uint8_t payload[PKTBUF_BLK_SIZE];    // 数据块的有效载荷大小

} pktblk_t;

/**
 * @brief 定义数据包结构
 * 
 */
typedef struct _pktbuf_t {
    nlist_node_t node;  // 数据包链表结点
    int total_size; // 数据包总大小
    nlist_t blk_list;   // 数据块链表

    int pos;    // 当前待读取字节在数据包的位置
    pktblk_t *curr_blk;  // 当前待读取字节所在的数据块
    uint8_t *curr_pos;  // 当前待读取字节的地址

    int ref_cnt;    // 引用计数
} pktbuf_t;


net_err_t pktbuf_init(void);
pktbuf_t *pktbuf_alloc(int size);
void pktbuf_free(pktbuf_t *pktbuf);

/**
 * 获取当前block的下一个子包
 */
static inline pktblk_t * pktbuf_blk_next(pktblk_t *blk) {
    nlist_node_t * next = nlist_node_next(&(blk->node));
    return nlist_entry(next, pktblk_t, node);
}

/**
 * 取buf的第一个数据块
 * @param buf 数据缓存buf
 * @return 第一个数据块
 */
static inline pktblk_t * pktbuf_blk_first(pktbuf_t * buf) {
    dbg_assert(buf->ref_cnt != 0, "buf ref_cnt == 0");
    nlist_node_t * first = nlist_first(&buf->blk_list);
    return nlist_entry(first, pktblk_t, node);
}

/**
 * @brief 取buf的最后一个数据块
 * @param buf buf 数据缓存buf
 * @return 最后一个数据块
 */
static inline pktblk_t * pktbuf_blk_last(pktbuf_t * buf) {

    dbg_assert(buf->ref_cnt != 0, "buf ref_cnt == 0");
    nlist_node_t * first = nlist_last(&buf->blk_list);
    return nlist_entry(first, pktblk_t, node);
}

/**
 * @brief 获取数据包的总大小
 * 
 * @param buf 
 * @return int 
 */
static inline  int pktbuf_total_size (pktbuf_t * buf) {

    dbg_assert(buf->ref_cnt != 0, "buf ref_cnt == 0");
    return buf->total_size;
}

/**
 * @brief 获取数据包起始数据的指针
 * 
 * @param buf 
 * @return uint8_t* 
 */
static inline uint8_t * pktbuf_data_ptr(pktbuf_t * buf) {
    dbg_assert(buf->ref_cnt != 0, "buf ref_cnt == 0");
    pktblk_t * first_blk = pktbuf_blk_first(buf);
    return first_blk ? first_blk->data : (uint8_t *)0;
}


net_err_t pktbuf_add_header(pktbuf_t * buf, int size, int is_cont);
net_err_t pktbuf_remove_header(pktbuf_t * buf, int size);
net_err_t pktbuf_resize (pktbuf_t * buf, int to_size);
net_err_t pktbuf_join(pktbuf_t * dest, pktbuf_t * src);
net_err_t pktbuf_set_cont(pktbuf_t *buf, int size);

void pktbuf_acc_reset(pktbuf_t * buf);

net_err_t pktbuf_write(pktbuf_t * buf, uint8_t * src, int size);
net_err_t pktbuf_read(pktbuf_t * buf, uint8_t * dest, int size);
net_err_t pktbuf_seek(pktbuf_t *buf, int offset);
net_err_t pktbuf_copy(pktbuf_t *dest, pktbuf_t *src, int size);
net_err_t pktbuf_fill(pktbuf_t *buf, uint8_t data, int size);


void pktbuf_inc_ref(pktbuf_t *buf);

#endif  // PKTBUF_H