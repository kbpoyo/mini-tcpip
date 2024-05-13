#include "netif_pcap.h"

#include "sys_plat.h"
#include "exmsg.h"

/**
 * @brief 网络包接收线程
 *
 * @param arg
 */
void recv_thread(void *arg) {
  printf("recv thread is start....\n");
  while (1) {
    exmsg_netif_in();
    sys_sleep(1);
   
  }
}

/**
 * @brief 网络包发送线程
 * 
 * @param arg 
 */
void xmit_thread(void *arg) {
  printf("xmit thread is start....\n");
  while (1) {
    printf("xmit_thread\n");
    sys_sleep(1000);
  }
}

/**
 * @brief pcap接口方式打开网卡
 *
 */
net_err_t netif_pcap_open(void) {
  sys_thread_create(recv_thread, NULL);
//   sys_thread_create(xmit_thread, NULL);

  return NET_ERR_OK;
}