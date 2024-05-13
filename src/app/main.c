#include <stdint.h>
#include <stdio.h>

#include "dbg.h"
#include "mblock.h"
#include "net.h"
#include "netif_pcap.h"
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

#define DBG_TEST DBG_LEVEL_INFO

int main(void) {
  net_init();

  net_start();

  netdev_init();

  // mblock_test();

  dbg_info(DBG_TEST, "debug info");
  dbg_warning(DBG_TEST, "debug waring");
  dbg_error(DBG_TEST, "debug error");

  dbg_assert(1 == 2, "assert error")

      while (1) {
    sys_sleep(10);
  }

  return 0;
}