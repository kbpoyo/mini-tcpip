/**
 * @file tcp_buf.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief tcp数据包缓冲区模块
 * @version 0.1
 * @date 2024-11-05
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef TCP_BUF_H
#define TCP_BUF_H

#include <stdint.h>
#include "pktbuf.h"

// 定义tcp数据buf结构
typedef struct _tcp_buf_t {
  uint8_t *data;  // 数据缓冲区
  int count;      // 数据缓冲区当前数据量
  int size;       // 数据缓冲区的容量
  int in, out;    // 数据缓冲区的读写索引

} tcp_buf_t;

void tcp_buf_init(tcp_buf_t *tcp_buf, uint8_t *data, int size);

static inline int tcp_buf_size(tcp_buf_t *tcp_buf) { return tcp_buf->size; }

static inline int tcp_buf_cnt(tcp_buf_t *tcp_buf) { return tcp_buf->count; }

static inline int tcp_buf_free_cnt(tcp_buf_t *tcp_buf) {
  return tcp_buf->size - tcp_buf->count;
}
int tcp_buf_write(tcp_buf_t *tcp_buf, const uint8_t *data_buf, int len);
int tcp_buf_read_to_pktbuf(tcp_buf_t *tcp_buf, pktbuf_t *buf, int offset, int len);
int tcp_buf_write_from_pktbuf(tcp_buf_t *tcp_buf, pktbuf_t *buf, int offset, int len);
int tcp_buf_read(tcp_buf_t *tcp_buf, uint8_t *data_buf, int len);

int tcp_buf_remove(tcp_buf_t *tcp_buf, int cnt);

#endif  // TCP_BUF_H