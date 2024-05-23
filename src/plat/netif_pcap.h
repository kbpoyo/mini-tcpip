#ifndef NETIF_PCAP_H
#define NETIF_PCAP_H

#include "net_err.h"
#include "pktbuf.h"
#include "netif.h"


/**
 * @brief 打开pcap所需的初始化数据
 * 
 */
typedef struct _pcap_data_t {
    const char *ip; // 需要打开的本地主机的网络适配器的ip地址
    const uint8_t *hwaddr; // 设置虚拟的网卡地址，通过打开的网络适配器进行数据包的收发

}pcap_data_t;



extern const netif_ops_t netif_pcap_ops;


#endif // NETIF_PCAP_H