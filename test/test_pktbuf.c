/**
 * @file test_pktbuf.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 数据包模块接口测试
 * @version 0.1
 * @date 2024-05-19
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "dbg.h"
#include "net_err.h"
#include "net_sys.h"
#include "pktbuf.h"

net_err_t test_pktbuf_alloc_and_free(void) {
  // 连续创建
  int alloc_cnt = 10000;
  for (int i = 0; i < alloc_cnt; i++) {
    pktbuf_t *buf = pktbuf_alloc(i);
    if (buf == NULL) {
      return NET_ERR_SYS;
    }

    pktbuf_free(buf);
    if (!nlist_is_empty(&(buf->blk_list)) || buf->ref_cnt != 0) {
      return NET_ERR_SYS;
    }
  }

  // 多次创建
  alloc_cnt = 1000;
  pktbuf_t *bufs[PKTBUF_BUF_CNT];
  int blk_size = 0;
  for (int i = 0; i < alloc_cnt; i++) {
    int base = 0;
    while ((blk_size + base * 100) < PKTBUF_BLK_CNT * PKTBUF_BLK_SIZE) {
      bufs[base] = pktbuf_alloc(base * 100);
      if (bufs[base] == NULL) {
        return NET_ERR_SYS;
      }

      blk_size += bufs[base]->total_size;
      base++;
    }

    for (int j = 0; j < base; j++) {
      blk_size -= bufs[j]->total_size;
      pktbuf_free(bufs[j]);
      if (!nlist_is_empty(&(bufs[j]->blk_list)) || bufs[j]->ref_cnt != 0) {
        return NET_ERR_SYS;
      }
    }

    if (blk_size != 0) {
      return NET_ERR_SYS;
    }
  }

  return NET_ERR_OK;
}

net_err_t test_pktbuf_head_add_and_remove(void) {
  pktbuf_t *buf = pktbuf_alloc(0);
  if (buf == NULL) {
    return NET_ERR_SYS;
  }

  net_err_t err = NET_ERR_OK;

  int buf_size = 0;
  for (int i = 0; i < PKTBUF_BLK_SIZE; i++) {  // 添加头部且确保连续

    err = pktbuf_header_add(buf, i, PKTBUF_ADD_HEADER_CONT);
    Net_Err_Check(err);
    buf_size += i;
  }

  if (buf->total_size != buf_size) {
    return NET_ERR_SYS;
  }

  for (int i = 0; i < PKTBUF_BLK_SIZE; i++) {  // 移除头部
    err = pktbuf_header_remove(buf, i);
    Net_Err_Check(err);
    buf_size -= i;
  }

  if (buf->total_size != 0) {
    return NET_ERR_SYS;
  }

  for (int i = 0; i < PKTBUF_BLK_SIZE; i++) {  // 添加100个头部且确保连续

    err = pktbuf_header_add(buf, i, PKTBUF_ADD_HEADER_UNCONT);
    Net_Err_Check(err);
    buf_size += i;
  }

  if (buf->total_size != buf_size) {
    return NET_ERR_SYS;
  }

  for (int i = 0; i < PKTBUF_BLK_SIZE; i++) {  // 移除100个头部
    err = pktbuf_header_remove(buf, i);
    Net_Err_Check(err);
    buf_size -= i;
  }

  if (buf->total_size != 0) {
    return NET_ERR_SYS;
  }

  pktbuf_free(buf);
  return NET_ERR_OK;
}

net_err_t test_pktbuf_read_and_write(void) {
  net_err_t err = NET_ERR_OK;
  int buf_size = PKTBUF_BLK_SIZE * 10;
  uint8_t src[PKTBUF_BLK_SIZE * 10];
  uint8_t dest[PKTBUF_BLK_SIZE * 10];
  for (int i = 0; i < buf_size; i++) {
    src[i] = i;
  }

  pktbuf_t *buf = pktbuf_alloc(buf_size * 2);
  if (buf == NULL) {
    return NET_ERR_SYS;
  }

  err = pktbuf_write(buf, src, buf_size);
  Net_Err_Check(err);

  pktbuf_acc_reset(buf);  // 重置数据包访问位置
  err = pktbuf_read(buf, dest, buf_size);
  Net_Err_Check(err);

  if (plat_memcmp(src, dest, buf_size) != 0) {
    return NET_ERR_SYS;
  }

  pktbuf_free(buf);
  return NET_ERR_OK;
}

net_err_t test_pktbuf_resize_and_join(void) {
  net_err_t err = NET_ERR_OK;

  pktbuf_t *buf = pktbuf_alloc(100);

  int buf_size = PKTBUF_BLK_SIZE * 10;
  uint8_t src[PKTBUF_BLK_SIZE * 10];
  uint8_t dest[PKTBUF_BLK_SIZE * 10];
  for (int i = 0; i < buf_size; i++) {
    src[i] = i;
  }

  // 测试1
  int write_size = buf_size;
  uint8_t *src_ptr = src;
  while (write_size > 0) {
    int remain_size = pktbuf_remain_size(buf);
    int size = write_size > remain_size ? remain_size : write_size;
    pktbuf_write(buf, src_ptr, size);
    write_size -= size;
    src_ptr += size;

    err = pktbuf_join(buf, pktbuf_alloc(100));
    Net_Err_Check(err);
  }
  pktbuf_acc_reset(buf);
  err = pktbuf_read(buf, dest, buf_size);
  Net_Err_Check(err);
  if (plat_memcmp(src, dest, buf_size) != 0) {
    return NET_ERR_SYS;
  }

  // 测试2
  plat_memset(dest, 0, buf_size);  // 清空dest
  pktbuf_resize(buf, buf_size / 2);
  pktbuf_acc_reset(buf);
  pktbuf_t *temp_buf = pktbuf_alloc(buf_size / 2);
  pktbuf_write(buf, src, buf_size / 2);
  pktbuf_write(temp_buf, src + buf_size / 2, buf_size / 2);

  err = pktbuf_join(buf, temp_buf);
  Net_Err_Check(err);

  pktbuf_acc_reset(buf);
  err = pktbuf_read(buf, dest, buf_size);
  Net_Err_Check(err);

  if (plat_memcmp(src, dest, buf_size) != 0) {
    return NET_ERR_SYS;
  }

  pktbuf_free(buf);
  return NET_ERR_OK;
}

net_err_t test_pktbuf_seek_and_copy(void) {
  pktbuf_t *buf = pktbuf_alloc(100);
  int buf_size = PKTBUF_BLK_SIZE * 10;
  uint8_t src[PKTBUF_BLK_SIZE * 10];
  uint8_t dest[PKTBUF_BLK_SIZE * 10];
  for (int i = 0; i < buf_size; i++) {
    src[i] = i;
  }

  pktbuf_resize(buf, buf_size);
  pktbuf_write(buf, src, buf_size);

  pktbuf_t *temp_buf = pktbuf_alloc(buf_size / 2);
  pktbuf_seek(buf, buf_size / 2);
  pktbuf_copy(temp_buf, buf, buf_size / 2);
  pktbuf_acc_reset(temp_buf);  // 重置数据包访问位置
  pktbuf_read(temp_buf, dest, buf_size / 2);
  if (plat_memcmp(src + buf_size / 2, dest, buf_size / 2) != 0) {
    dbg_error(DBG_PKTBUF, "test_pktbuf_seek_copy failed.");
  }

  pktbuf_free(buf);
  pktbuf_free(temp_buf);

  return NET_ERR_OK;
}

net_err_t test_pktbuf_setcont_and_fill(void) {
  // 测试set_cont
  pktbuf_t *pktbuf = pktbuf_alloc(32);
  pktbuf_join(pktbuf, pktbuf_alloc(4));
  pktbuf_join(pktbuf, pktbuf_alloc(16));
  pktbuf_join(pktbuf, pktbuf_alloc(54));
  pktbuf_join(pktbuf, pktbuf_alloc(32));
  pktbuf_join(pktbuf, pktbuf_alloc(38));

  pktblk_t *blk = pktbuf_blk_first(pktbuf);
  if (blk->data_size != 32) {
    return NET_ERR_SYS;
  }


  pktbuf_set_cont(pktbuf, 44);
  if (blk->data_size < 44) {
    return NET_ERR_SYS;
  }
  pktbuf_set_cont(pktbuf, 60);
  if (blk->data_size < 60) {
    return NET_ERR_SYS;
  }
  pktbuf_set_cont(pktbuf, 44);
  if (blk->data_size < 44) {
    return NET_ERR_SYS;
  }
  pktbuf_set_cont(pktbuf, 128);
  if (blk->data_size < 128) {
    return NET_ERR_SYS;
  }

  // 测试fill
  pktbuf_resize(pktbuf, PKTBUF_BLK_SIZE * 10);
  pktbuf_acc_reset(pktbuf);
  pktbuf_fill(pktbuf, 0x55, pktbuf_total_size(pktbuf));
  pktbuf_acc_reset(pktbuf);
  uint8_t data[PKTBUF_BLK_SIZE * 10];
  pktbuf_read(pktbuf, data, pktbuf_total_size(pktbuf));
  for (int i = 0; i < pktbuf_total_size(pktbuf); i++) {
    if (data[i] != 0x55) {
      return NET_ERR_SYS;
    }
  }

  pktbuf_free(pktbuf);
  return NET_ERR_OK;
}

int main(void) {
  pktbuf_module_init();
  net_err_t err = NET_ERR_OK;

  err = test_pktbuf_alloc_and_free();
  if (err != NET_ERR_OK) {
    dbg_error(DBG_PKTBUF, "test_pktbuf_alloc_and_free failed.");
    return -1;
  }

  err = test_pktbuf_head_add_and_remove();
  if (err != NET_ERR_OK) {
    dbg_error(DBG_PKTBUF, "test_pktbuf_head_add_and_remove failed.");
    return -1;
  }

  err = test_pktbuf_read_and_write();
  if (err != NET_ERR_OK) {
    dbg_error(DBG_PKTBUF, "test_pktbuf_read_and_write failed.");
    return -1;
  }

  err = test_pktbuf_resize_and_join();
  if (err != NET_ERR_OK) {
    dbg_error(DBG_PKTBUF, "test_pktbuf_resize_and_join failed.");
    return -1;
  }

  err = test_pktbuf_seek_and_copy();
  if (err != NET_ERR_OK) {
    dbg_error(DBG_PKTBUF, "test_pktbuf_seek_and_copy failed.");
    return -1;
  }

  err = test_pktbuf_setcont_and_fill();
  if (err != NET_ERR_OK) {
    dbg_error(DBG_PKTBUF, "test_pktbuf_setcont_and_fill failed.");
    return -1;
  }

  return 0;
}