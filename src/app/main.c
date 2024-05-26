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

  return NET_ERR_OK;
}

void basic_test(void) {
  uint16_t v1 = net_htons(0x1234);
  uint32_t v2 = net_htonl(0x12345678);
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