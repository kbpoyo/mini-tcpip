/**
 * @file tcp_buf.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief tcp数据包缓冲区模块, 采用环形缓冲队列
 * TODO: 之后可用内核的kfifo来进行优化
 * @version 0.1
 * @date 2024-11-05
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "tcp_buf.h"

#include "sys_plat.h"
#include "tools.h"
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
 * @brief 实际完成向tcp_buf中写入数据的操作
 *
 * @param tcp_buf
 * @param data_buf
 * @param len
 * @return int
 */
static void _tcp_buf_write(tcp_buf_t *tcp_buf, const uint8_t *data_buf,
                           int len) {
  // 计算剩余空间并拷贝，情况1：可用空间未形成回绕，cpy_len = len,
  // 只进行第一次拷贝 情况2：可用空间形成回绕，cpy_len = tcp_buf->size -
  // tcp_buf->in, 还需要进行第二次拷贝(len - cpy_len)
  int cpy_len = MIN(len, tcp_buf->size - tcp_buf->in);

  plat_memcpy(&tcp_buf->data[tcp_buf->in], data_buf, cpy_len);
  plat_memcpy(tcp_buf->data, data_buf + cpy_len, len - cpy_len);
}

/**
 * @brief 向tcp_buf中写入数据
 *
 * @param tcp_buf
 * @param data_buf 待写入的数据首地址
 * @param len 待写入的数据长度
 * @return int 成功写入的数据长度
 */
int tcp_buf_write(tcp_buf_t *tcp_buf, const uint8_t *data_buf, int len) {
  int free_cnt = tcp_buf_free_cnt(tcp_buf);
  if (free_cnt <= 0) {
    return 0;
  }

  // 根据剩余空间大小，写入数据到缓冲区
  int write_len = MIN(len, free_cnt);
  _tcp_buf_write(tcp_buf, data_buf, write_len);

  // 更新缓冲区的数据量和读写索引
  tcp_buf->count += write_len;
  tcp_buf->in = (tcp_buf->in + write_len) % tcp_buf->size;
  return write_len;
}

/**
 * @brief 从tcp_buf(tcp发送缓冲区)中读取数据到pktbuf(tcp数据包)中
 *      从缓冲区out指针偏移offset字节处开始读取数据
 *
 * @param tcp_buf
 * @param buf
 * @param offset 待读取数据相对于缓冲区out指针的偏移量
 * @param len 待读取数据长度
 * @return int 读取的数据长度
 */
int tcp_buf_read_to_pktbuf(tcp_buf_t *tcp_buf, pktbuf_t *buf, int offset, int len) {
  net_err_t err = NET_ERR_OK;

  // 计算待读取数据的起始索引
  int start_idx = (tcp_buf->out + offset) % tcp_buf->size;

  // 计算可读取的数据长度，情况1：可读数据未形成回绕，cpy_len = len,
  // 只进行第一次拷贝 情况2：可读数据形成回绕，cpy_len = tcp_buf->size -
  // start_idx, 还需要进行第二次拷贝(len - cpy_len)
  int cpy_len = MIN(len, tcp_buf->size - start_idx);
  err = pktbuf_write(buf, &tcp_buf->data[start_idx], cpy_len);
  err = pktbuf_write(buf, tcp_buf->data, len - cpy_len);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_TCP, "send buf read failed, pktbuf write error.");
    return -1;
  }

  return len;
}

/**
 * @brief 从tcp_buf队列中移除cnt个数据
 *
 * @param tcp_buf
 * @param cnt
 * @return int 成功移除的数据长度
 */
int tcp_buf_remove(tcp_buf_t *tcp_buf, int cnt) {
  cnt = MIN(cnt, tcp_buf->count);
  tcp_buf->out = (tcp_buf->out + cnt) % tcp_buf->size;
  tcp_buf->count -= cnt;
  
  return cnt;
}