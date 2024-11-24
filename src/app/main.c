#include <stdint.h>
#include <stdio.h>

#include "dbg.h"
#include "echo/tcp_echo_client.h"
#include "echo/udp_echo_client.h"
#include "echo/udp_echo_server.h"
#include "exmsg.h"
#include "ipv4.h"
#include "mblock.h"
#include "net.h"
#include "netif.h"
#include "netif_pcap.h"
#include "ping/ping.h"
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
  ipaddr_from_str(&ip, netdev0_ip);
  ipaddr_from_str(&mask, netdev0_mask);
  ipaddr_from_str(&gw, netdev0_gw);
  netif_set_addr(netif, &ip, &mask, &gw);

  // 设置接口为激活态
  err = netif_set_acticve(netif);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_LOOP, "netif set active failed.");
    return err;
  }

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

void udp_echo_test() {
  // udp_echo_server_start(2000);

  // udp_echo_client_start("192.168.3.159", 2000);
  // udp_echo_client_start("0.0.0.0", 1024);
}

void tcp_echo_test() { tcp_echo_client_start("192.168.3.159", 2000); }

void tcp_download_test() { download_test("hello.txt", 2000); }
void basic_test(void) {
  // timer_test();

  // udp_echo_test();
  // tcp_echo_test();

  tcp_download_test();
}

#define DBG_TEST DBG_LEVEL_INFO

net_err_t test_func(msg_func_t *msg) {
  plat_printf("test hello world 0x%x\n", *(int *)msg->arg);
  return NET_ERR_OK;
}

int main(void) {
  net_init();

  net_start();

  netdev_init();
  basic_test();


  // 简单的命令行测试
  char cmd[32], param[32];
  ping_t ping;
  while (1) {
    // plat_printf(">>");
    // scanf("%s%s", cmd, param);
    // if (strcmp(cmd, "ping") == 0) {
    //   ping_run(&ping, param, 1024, 4, 1000);
    // }

    sys_sleep(10);
  }

  return 0;
}