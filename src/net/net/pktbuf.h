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

#include "dbg.h"
#include "net_cfg.h"
#include "net_err.h"
#include "nlist.h"

#define PKTBUF_LIST_INSERT_HEAD 0   // 使用头插法
#define PKTBUF_LIST_INSERT_TAIL 1   // 使用尾插法
#define PKTBUF_ADD_HEADER_UNCONT 0  // 增添数据块时不连续
#define PKTBUF_ADD_HEADER_CONT 1    // 增添数据块时连续

/**
 * @brief 定义数据块结构
 *
 */
typedef struct _pktblk_t {
  nlist_node_t
      node;  // 数据块链表结点, 放在第一个元素的位置，方便调试观察在哪个列表中
  int data_size;                     // 数据大小
  uint8_t *data;                     // 数据起始地址
  uint8_t payload[PKTBUF_BLK_SIZE];  // 数据块的有效载荷大小

} pktblk_t;

/**
 * @brief 定义数据包结构
 * 数据包的生命周期为：
 *  1. 函数Fun1通过pktbuf_alloc()分配一个数据包buf,
 * 或通过一个缓存的buf_list中取出一个buf (Fun1对buf负责)
 *  2.
 * 若Fun1需要传递buf给Fun2，且Fun2函数执行成功，则由Fun2负责调用pktbuf_free()释放buf
 * (fun2对buf负责)
 *  3.
 * 若Fun1需要传递buf给Fun2，且Fun2函数执行失败，则由Fun1负责调用pktbuf_free()释放buf
 * (fun1对buf负责)
 *  4. Fun1到Fun2的传播链路中的其它函数，不需要调用pktbuf_free()释放buf
 * (链式传递的其它函数不对buf负责)
 */
typedef struct _pktbuf_t {
  nlist_node_t node;  // 数据包链表结点
  int total_size;     // 数据包总大小
  nlist_t blk_list;   // 数据块链表

  int pos;             // 当前待读取字节在数据包的位置
  pktblk_t *curr_blk;  // 当前待读取字节所在的数据块
  uint8_t *curr_pos;   // 当前待读取字节的地址

  int ref_cnt;  // 引用计数
} pktbuf_t;

#define pktbuf_check_buf(buf)                             \
  {                                                       \
    dbg_assert(buf != (pktbuf_t *)0 && buf->ref_cnt != 0, \
               "buf = 0 or buf ref = 0.");                \
  }

/**
 * 获取当前block的下一个子包
 */
static inline pktblk_t *pktbuf_blk_next(pktbuf_t *pktbuf, pktblk_t *blk) {
  nlist_node_t *next = nlist_next(&pktbuf->blk_list, &blk->node);
  return nlist_entry(next, pktblk_t, node);
}

/**
 * 取buf的第一个数据块
 * @param buf 数据缓存buf
 * @return 第一个数据块
 */
static inline pktblk_t *pktbuf_blk_first(pktbuf_t *buf) {
  pktbuf_check_buf(buf);
  nlist_node_t *first = nlist_first(&buf->blk_list);
  return nlist_entry(first, pktblk_t, node);
}

/**
 * @brief 取buf的最后一个数据块
 * @param buf buf 数据缓存buf
 * @return 最后一个数据块
 */
static inline pktblk_t *pktbuf_blk_last(pktbuf_t *buf) {
  pktbuf_check_buf(buf);
  nlist_node_t *first = nlist_last(&buf->blk_list);
  return nlist_entry(first, pktblk_t, node);
}

/**
 * @brief 获取数据包的总大小
 *
 * @param buf
 * @return int
 */
static inline int pktbuf_total_size(pktbuf_t *buf) {
  pktbuf_check_buf(buf);
  return buf->total_size;
}

/**
 * @brief 获取数据包起始数据的指针
 *
 * @param buf
 * @return void*
 */
static inline void *pktbuf_data_ptr(pktbuf_t *buf) {
  pktbuf_check_buf(buf);
  pktblk_t *first_blk = pktbuf_blk_first(buf);
  return first_blk ? first_blk->data : (void *)0;
}

/**
 * @brief 获取数据包剩余待读取字节数
 *
 * @param buf
 * @return int
 */
static inline int pktbuf_remain_size(const pktbuf_t *buf) {
  return buf->total_size - buf->pos;
}

net_err_t pktbuf_module_init(void);
pktbuf_t *pktbuf_alloc(int size);
void pktbuf_free(pktbuf_t *buf);
net_err_t pktbuf_resize(pktbuf_t *buf, int to_size);
net_err_t pktbuf_join(pktbuf_t *dest, pktbuf_t *src);

net_err_t pktbuf_header_add(pktbuf_t *buf, int size, int is_cont);
net_err_t pktbuf_header_remove(pktbuf_t *buf, int size);

net_err_t pktbuf_set_cont(pktbuf_t *buf, int size);

void pktbuf_acc_reset(pktbuf_t *buf);
net_err_t pktbuf_write(pktbuf_t *buf, const uint8_t *src, int size);
net_err_t pktbuf_read(pktbuf_t *buf, uint8_t *dest, int size);
net_err_t pktbuf_seek(pktbuf_t *buf, int offset);
net_err_t pktbuf_copy(pktbuf_t *dest, pktbuf_t *src, int size);
net_err_t pktbuf_fill(pktbuf_t *buf, uint8_t data, int size);
pktbuf_t *pktbuf_inc_ref(pktbuf_t *buf);


uint16_t pktbuf_checksum16(pktbuf_t *buf, uint16_t size, uint32_t pre_sum,
                           int is_take_back);

// 外部函数在传递pktbuf_t指针时，使用PKTBUF_ARG宏,
// 每一个拥有pktbuf_t指针的函数在完成后，都需要对pktbuf进行释放
#define PKTBUF_ARG(buf) pktbuf_inc_ref(buf)

#endif  // PKTBUF_H