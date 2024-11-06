/**
 * @file tcp_buf.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief tcp数据包缓冲区模块
 * @version 0.1
 * @date 2024-11-05
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "tcp_buf.h"

/**
 * @brief 初始化tcp_buf对象
 *
 * @param tcp_buf
 * @param data buf的首地址
 * @param size buf的大小
 */
void tcp_buf_init(tcp_buf_t *tcp_buf, uint8_t *data, int size) {
  tcp_buf->data = data;
  tcp_buf->count = 0;
  tcp_buf->size = size;
  tcp_buf->in = tcp_buf->out = 0;
}

/**
 * @brief 将数据写入tcp_buf
 *
 * @param tcp_buf 代写入的tcp缓冲区
 * @param data_buf 写入的数据
 * @param len 写入数据的长度
 * @return int
 */
void tcp_buf_write(tcp_buf_t *tcp_buf, const uint8_t *data_buf, int len) {
  while (len > 0) {
    tcp_buf->data[tcp_buf->in++] = *(data_buf++);
    tcp_buf->in =
        tcp_buf->in >= tcp_buf->size ? 0 : tcp_buf->in;  // 使用环形缓冲区
    len--;
    tcp_buf->count++;  // buf中有效数据量增加
  }
}

/**
 * @brief 从tcp_buf(tcp发送缓冲区)中读取数据到pktbuf(tcp数据包)中
 * 
 * @param tcp_buf 
 * @param data 数据在缓冲区中的信息结构
 * @param buf 
 * @return int 
 */
int tcp_buf_read_to_pktbuf(tcp_buf_t *tcp_buf, tcp_data_t *data, pktbuf_t *buf) {
  net_err_t err = NET_ERR_OK;
  
  // 获取待读取数据的起始索引和长度
  int start_idx = data->start_idx;
  int len = data->len;

  // 从发送缓冲区中读取数据到tcp数据包中
  while (len > 0) {
    // 计算可拷贝的数据长度
    int end_idx = start_idx + len;
    end_idx = end_idx >= tcp_buf->size ? tcp_buf->size : end_idx; // 回绕处理
    int copy_len = end_idx - start_idx;

    // 将数据拷贝到tcp数据包中
    err = pktbuf_write(buf, &tcp_buf->data[start_idx], copy_len);
    if (err != NET_ERR_OK) {
      dbg_error(DBG_TCP, "send buf read failed, pktbuf write error.");
      return (data->len - len);
    }

    // 更新待读取数据的起始索引和长度
    start_idx += copy_len;
    start_idx = start_idx >= tcp_buf->size ? 0 : start_idx;
    len -= copy_len;
  }

  return (data->len - len);
}
