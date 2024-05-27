#include <stdint.h>
#include <stdio.h>

#include "dbg.h"
#include "mblock.h"
#include "net.h"
#include "netif.h"
#include "netif_pcap.h"
#include "pktbuf.h"
#include "sys_plat.h"
#include "tools.h"
#include "timer.h"

pcap_data_t pcap_data = {
    // 需要打开的网络接口的ip地址和硬件地址
    .ip = netdev0_phy_ip,
    .hwaddr = netdev0_hwaddr,
};

net_err_t netdev_init(void) {
  // 打开一个pcap网络接口
  netif_t *netif = netif_open("pcap 0", &netif_pcap_ops, &pcap_data);
  if (netif == (netif_t *)0) {
    dbg_error(DBG_LOOP, "netif open failed.");
    return NET_ERR_MEM;
  }

  // 设置接口的ip地址和子网掩码
  ipaddr_t ip, mask, gw;
  ipaddr_from_str(&ip, netdev0_ip);
  ipaddr_from_str(&mask, netdev0_mask);
  ipaddr_from_str(&gw, netdev0_gw);
  netif_set_addr(netif, &ip, &mask, &gw);

  // 设置接口为激活态
  netif_set_acticve(netif);

  pktbuf_t *buf = pktbuf_alloc(32);
  pktbuf_fill(buf, 0x53, 32);
  netif_send(netif, (ipaddr_t*)0, buf);

  return NET_ERR_OK;
}

void timer0_handle(net_timer_t *timer, void *arg) {
  static int count0 = 1;
  dbg_info(DBG_TIMER, "thist is %s: %d\n", timer->name, count0++);
}
void timer1_handle(net_timer_t *timer, void *arg) {
  static int count1 = 1;
  dbg_info(DBG_TIMER, "thist is %s: %d\n", timer->name, count1++);
}
void timer2_handle(net_timer_t *timer, void *arg) {
  static int count2 = 1;
  dbg_info(DBG_TIMER, "thist is %s: %d\n", timer->name, count2++);
}
void timer3_handle(net_timer_t *timer, void *arg) {
  static int count3 = 1;
  dbg_info(DBG_TIMER, "thist is %s: %d\n", timer->name, count3++);
}

void timer_test() {
  static net_timer_t t0, t1, t2, t3;

  net_timer_add(&t0, "t0", timer0_handle, 0, 200, 0);

  net_timer_add(&t1, "t1", timer1_handle, 0, 1000, NET_TIMER_RELOAD);
  net_timer_add(&t2, "t2", timer2_handle, 0, 1000, NET_TIMER_RELOAD);
  net_timer_add(&t3, "t3", timer3_handle, 0, 4000, NET_TIMER_RELOAD);
}


void basic_test(void) {
  timer_test();
}

#define DBG_TEST DBG_LEVEL_INFO

int main(void) {
  net_init();

  netdev_init();

  net_start();

  basic_test();

  while (1) {
    sys_sleep(10);
  }

  return 0;
}