﻿#include <stdint.h>
#include <stdio.h>

#include "dbg.h"
#include "mblock.h"
#include "net.h"
#include "netif_pcap.h"
#include "pktbuf.h"
#include "sys_plat.h"

net_err_t netdev_init(void) {
  netif_pcap_open();

  return NET_ERR_OK;
}

void mblock_test(void) {
  mblock_t blist;
  static uint8_t buf[10][100];
  mblock_init(&blist, buf, 100, 10, NLOCKER_THREAD);

  void *temp[10];
  for (int i = 0; i < 10; i++) {
    temp[i] = mblock_alloc(&blist, 0);
    plat_printf("block: %p, free count: %d\n", temp[i],
                mblock_free_cnt(&blist));
  }

  for (int i = 0; i < 10; i++) {
    mblock_free(&blist, temp[i]);
    plat_printf("free block: %p, free count: %d\n", temp[i],
                mblock_free_cnt(&blist));
  }

  mblock_destroy(&blist);
}

void pktbuf_test() {
  pktbuf_t *pktbuf = pktbuf_alloc(10000);
  pktbuf_free(pktbuf);

  // 测试包头添加和删除函数 pktbuf_add_header, pktbuf_remove_header
  // pktbuf = pktbuf_alloc(5000);
  // for (int i = 0; i < 16; i++) {
  //   pktbuf_add_header(pktbuf, 33, PKTBUF_ADD_HEADER_CONT);
  // }

  // for (int i = 0; i < 16; i++) {
  //   pktbuf_remove_header(pktbuf, 33);
  // }

  // for (int i = 0; i < 16; i++) {
  //   pktbuf_add_header(pktbuf, 33, PKTBUF_ADD_HEADER_UNCONT);
  // }

  // for (int i = 0; i < 16; i++) {
  //   pktbuf_remove_header(pktbuf, 33);
  // }

  // pktbuf_free(pktbuf);

  // // 测试包大小调整函数 pktbuf_resize
  // pktbuf = pktbuf_alloc(8);
  // pktbuf_resize(pktbuf, 32);
  // pktbuf_resize(pktbuf, 288);
  // pktbuf_resize(pktbuf, 4922);

  // pktbuf_resize(pktbuf, 32);
  // pktbuf_resize(pktbuf, 0);
  // pktbuf_free(pktbuf);

  // // 测试包合并函数 pktbuf_join
  // pktbuf = pktbuf_alloc(689);
  // pktbuf_t *srcbuf = pktbuf_alloc(892);
  // pktbuf_join(pktbuf, srcbuf);
  // pktbuf_free(pktbuf);

  // // 测试包连续设置函数 ptkbuf_set_cont
  // pktbuf = pktbuf_alloc(32);
  // pktbuf_join(pktbuf, pktbuf_alloc(4));
  // pktbuf_join(pktbuf, pktbuf_alloc(16));
  // pktbuf_join(pktbuf, pktbuf_alloc(54));
  // pktbuf_join(pktbuf, pktbuf_alloc(32));
  // pktbuf_join(pktbuf, pktbuf_alloc(38));

  // pktbuf_set_cont(pktbuf, 44);
  // pktbuf_set_cont(pktbuf, 60);
  // pktbuf_set_cont(pktbuf, 44);
  // pktbuf_set_cont(pktbuf, 128);
  // // pktbuf_set_cont(pktbuf, 135);

  // pktbuf_free(pktbuf);

  // 测试包访问位置重置函数 pktbuf_acc_reset
  pktbuf = pktbuf_alloc(965);
  pktbuf_acc_reset(pktbuf);

  // 测试包读取与写入函数 pktbuf_write pktbuf_read
  static uint16_t temp_wirte[1000];
  for (int i = 0; i < 1000; i++) {
    temp_wirte[i] = i;
  }

  pktbuf_write(pktbuf, (uint8_t *)temp_wirte, pktbuf_total_size(pktbuf));

  static uint16_t temp_read[1000];
  plat_memset(temp_read, 0, sizeof(temp_read));

  pktbuf_acc_reset(pktbuf); // 重置访问位置
  pktbuf_read(pktbuf, (uint8_t *)temp_read, pktbuf_total_size(pktbuf));

  if (plat_memcmp(temp_wirte, temp_read, pktbuf_total_size(pktbuf)) != 0) {
    plat_printf("pktbuf write and read test failed.\n");
  }


  // 测试包访问位置偏移函数 pktbuf_seek
  plat_memset(temp_read, 0, sizeof(temp_read));
  pktbuf_seek(pktbuf, 18 * 2);
  pktbuf_read(pktbuf, (uint8_t *)temp_read, 56);
  if (plat_memcmp(temp_wirte + 18, temp_read, 56) != 0) {
    plat_printf("pktbuf seek test failed.\n");
  }

  plat_memset(temp_read, 0, sizeof(temp_read));
  pktbuf_seek(pktbuf, 85 * 2);
  pktbuf_read(pktbuf, (uint8_t *)temp_read, 256);
  if (plat_memcmp(temp_wirte + 85, temp_read, 256) != 0) {
    plat_printf("pktbuf seek test failed.\n");
  }

  // 测试包拷贝函数 pktbuf_copy
  pktbuf_t *dest = pktbuf_alloc(1024);
  pktbuf_seek(dest, 600);
  pktbuf_seek(pktbuf, 200);
  pktbuf_copy(dest, pktbuf, 122);

  plat_memset(temp_read, 0, sizeof(temp_read));
  pktbuf_seek(dest, 600);
  pktbuf_read(dest, (uint8_t *)temp_read, 122);
  if (plat_memcmp(temp_wirte + 100, temp_read, 122) != 0) {
    plat_printf("pktbuf copy test failed.\n");
  }

  pktbuf_free(pktbuf);
  pktbuf_free(dest);

  // 测试数据包填充函数 pktbuf_fill
  pktbuf = pktbuf_alloc(1024);
  pktbuf_fill(pktbuf, 55, pktbuf_total_size(pktbuf));
  pktbuf_acc_reset(pktbuf);
  pktbuf_read(pktbuf, (uint8_t *)temp_read, pktbuf_total_size(pktbuf));
  for (int i = 0; i < pktbuf_total_size(pktbuf); i++) {
    if (((uint8_t*)temp_read)[i] != 55) {
      plat_printf("pktbuf fill test failed.\n");
      break;
    }
  }

}

void basic_test(void) {
  // mblock_test();
  pktbuf_test();
}

#define DBG_TEST DBG_LEVEL_INFO

int main(void) {
  net_init();

   net_start();

  // netdev_init();

  basic_test();

  // dbg_info(DBG_TEST, "debug info");
  // dbg_warning(DBG_TEST, "debug waring");
  // dbg_error(DBG_TEST, "debug error");

  // dbg_assert(1 == 2, "assert error")

  while (1) {
    sys_sleep(10);
  }

  return 0;
}