#include <stdint.h>
#include <stdio.h>

#include "dbg.h"
#include "mblock.h"
#include "net.h"
#include "netif.h"
#include "netif_pcap.h"
#include "pktbuf.h"
#include "sys_plat.h"
#include "timer.h"
#include "tools.h"

pcap_data_t pcap_data = {
    // 需要打开的网络接口的ip地址和硬件地址
    .ip = netdev0_phy_ip,
    .hwaddr = netdev0_hwaddr,
};

net_err_t netdev_init(void) {
  net_err_t err = NET_ERR_OK;
  // 打开一个pcap网络接口
  netif_t *netif = netif_open("pcap 0", &netif_pcap_ops, &pcap_data);
  if (netif == (netif_t *)0) {
    dbg_error(DBG_LOOP, "netif open failed.");
    return NET_ERR_MEM;
  }

  // 设置接口的ip地址和子网掩码
  ipaddr_t ip, mask, gw;
  ipaddr_from_str(&ip, "192.168.74.2");
  // ipaddr_from_str(&ip, netdev0_ip);
  ipaddr_from_str(&mask, netdev0_mask);
  ipaddr_from_str(&gw, netdev0_gw);
  netif_set_addr(netif, &ip, &mask, &gw);

  // 设置接口为激活态
  err = netif_set_acticve(netif);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_LOOP, "netif set active failed.");
    return err;
  }

  pktbuf_t *buf = pktbuf_alloc(32);
  pktbuf_fill(buf, 0x53, 32);

  ipaddr_t dest;
  ipaddr_from_str(&dest, "192.168.74.3");
  netif_send(netif, &dest, buf);

  return NET_ERR_OK;
}

void timer0_handle(net_timer_t *timer, void *arg) {
  static int count0 = 1;
  plat_printf("thist is %s: %d\n", timer->name, count0++);
}
void timer1_handle(net_timer_t *timer, void *arg) {
  static int count1 = 1;
  plat_printf("thist is %s: %d\n", timer->name, count1++);
}
void timer2_handle(net_timer_t *timer, void *arg) {
  static int count2 = 1;
  plat_printf("thist is %s: %d\n", timer->name, count2++);
}
void timer3_handle(net_timer_t *timer, void *arg) {
  static int count3 = 1;
  plat_printf("thist is %s: %d\n", timer->name, count3++);
}
void timer4_handle(net_timer_t *timer, void *arg) {
  static int count4 = 1;
  plat_printf("thist is %s: %d\n", timer->name, count4++);
}

void timer_test() {
  static net_timer_t t0, t1, t2, t3, t4;

  net_timer_add(&t0, "t0", timer0_handle, 0, 1000, NET_TIMER_ACTIVE);

  net_timer_add(&t1, "t1", timer1_handle, 0, 1000,
                NET_TIMER_RELOAD | NET_TIMER_ACTIVE);
  net_timer_add(&t2, "t2", timer2_handle, 0, 2000,
                NET_TIMER_RELOAD | NET_TIMER_ACTIVE);
  net_timer_add(&t3, "t3", timer3_handle, 0, 3000,
                NET_TIMER_RELOAD | NET_TIMER_ACTIVE);
  net_timer_add(&t4, "t4", timer4_handle, 0, 4000,
                NET_TIMER_RELOAD | NET_TIMER_ACTIVE);

  // net_timer_remove(&t0);

  // net_timer_check_tmo(100);
  // net_timer_check_tmo(1200);
  // net_timer_check_tmo(6000);
}

void basic_test(void) {
  // timer_test();
}

#define DBG_TEST DBG_LEVEL_INFO

int main(void) {
  net_init();

  net_start();

  netdev_init();
  basic_test();

  while (1) {
    sys_sleep(10);
  }

  return 0;
}