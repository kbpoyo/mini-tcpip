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
    return free_cnt;
  }

  // 根据剩余空间大小，写入数据到缓冲区
  int write_len = MIN(len, free_cnt);
  _tcp_buf_write(tcp_buf, data_buf, write_len);

  // 更新缓冲区的数据量和读写索引
  tcp_buf->count += write_len;
  tcp_buf->in = (tcp_buf->in + write_len) % tcp_buf->size;
  return write_len;
}

static void _tcp_buf_read(tcp_buf_t *tcp_buf, uint8_t *data_buf, int len) {
  // 计算剩可读数据并拷贝，情况1：可读数据未形成回绕，cpy_len = len,
  // 只进行第一次拷贝 情况2：可读数据形成回绕，cpy_len = tcp_buf->size -
  // tcp_buf->out, 还需要进行第二次拷贝(len - cpy_len)
  int cpy_len = MIN(len, tcp_buf->size - tcp_buf->out);
  plat_memcpy(data_buf, &tcp_buf->data[tcp_buf->out], cpy_len);
  plat_memcpy(data_buf + cpy_len, tcp_buf->data, len - cpy_len);
}

/**
 * @brief 从tcp_buf中读取数据到data_buf中
 *
 * @param tcp_buf
 * @param data_buf
 * @param len
 * @return int
 */
int tcp_buf_read(tcp_buf_t *tcp_buf, uint8_t *data_buf, int len) {
  if (tcp_buf->count <= 0) {
    return tcp_buf->count;
  }

  // 根据可读数据量，读取数据到buf中
  int read_len = MIN(len, tcp_buf->count);
  _tcp_buf_read(tcp_buf, data_buf, read_len);

  // 更新缓冲区的数据量和读写索引
  tcp_buf->count -= read_len;
  tcp_buf->out = (tcp_buf->out + read_len) % tcp_buf->size;

  return read_len;
}

static net_err_t _tcp_buf_read_to_pktbuf(tcp_buf_t *tcp_buf, pktbuf_t *buf, int offset,
                                 int len) {
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
    dbg_error(DBG_TCP, "pktbuf write error.");
  }
  return err;
}

/**
 * @brief 从tcp_buf(tcp发送缓冲区)中读取数据到pktbuf(tcp数据包)中
 *      从缓冲区out指针偏移offset字节处开始读取数据，且不移除读取的数据
 *
 * @param tcp_buf
 * @param buf
 * @param offset 待读取数据相对于缓冲区out指针的偏移量
 * @param len 需要确保len <= tcp_buf->count， 内部不做检查
 * @return int 读取的数据长度
 */
int tcp_buf_read_to_pktbuf(tcp_buf_t *tcp_buf, pktbuf_t *buf, int offset,
                           int len) {
  net_err_t err = NET_ERR_OK;
  // 判断是否有数据可读
  if (tcp_buf->count <= 0) {
    return tcp_buf->count;
  }

  // 根据可读数据量，读取数据到buf中
  int read_len = MIN(len, tcp_buf->count);
  err = _tcp_buf_read_to_pktbuf(tcp_buf, buf, offset, read_len);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_TCP, "tcp buf read to pktbuf failed.");
    return -1;
  }

  return read_len;
}

/**
 * @brief 从pktbuf中读取数据到tcp_buf中
 *
 * @param tcp_buf
 * @param buf
 * @param offset
 * @param len 需要确保len <= tcp_buf->size - tcp_buf->count， 内部不做检查
 * @return int
 */
static net_err_t _tcp_buf_write_from_pktbuf(tcp_buf_t *tcp_buf, pktbuf_t *buf,
                                            int offset, int len) {
  net_err_t err = NET_ERR_OK;
  // 计算待写入数据的起始索引
  int start_idx = (tcp_buf->in + offset) % tcp_buf->size;
  // 计算可写入的数据长度，情况1：预写入空闲区未形成回绕，cpy_len = len,
  // 只进行第一次拷贝 情况2：可写数据形成回绕，cpy_len = tcp_buf->size -
  // start_idx, 还需要进行第二次拷贝(len - cpy_len)
  int cpy_len = MIN(len, tcp_buf->size - start_idx);
  err = pktbuf_read(buf, &tcp_buf->data[start_idx], cpy_len);
  err = pktbuf_read(buf, tcp_buf->data, len - cpy_len);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_TCP, "pktbuf read error.");
  }

  return err;
}

/**
 * @brief 将pktbuf中的数据写入到tcp_buf中
 *
 * @param tcp_buf
 * @param buf
 * @param offset 相对于缓冲区in指针的偏移量
 * @param len
 * @return int
 */
int tcp_buf_write_from_pktbuf(tcp_buf_t *tcp_buf, pktbuf_t *buf, int offset,
                              int len) {
  net_err_t err = NET_ERR_OK;

  // 判断是否有空间可写
  int free_cnt = tcp_buf_free_cnt(tcp_buf) - offset;
  if (free_cnt <= 0) {
    return free_cnt;
  }

  // 根据剩余空间大小，写入数据到缓冲区
  int write_len = MIN(len, free_cnt);
  err = _tcp_buf_write_from_pktbuf(tcp_buf, buf, offset, write_len);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_TCP, "pktbuf read error.");
    return -1;
  }

  // 更新缓冲区的数据量和读写索引
  tcp_buf->count += write_len;
  tcp_buf->in = (tcp_buf->in + write_len) % tcp_buf->size;
  return write_len;
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