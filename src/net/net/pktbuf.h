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



#endif  // PKTBUF_H