/**
 * @file netif_pcap.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 使用pcap库模拟网卡驱动，并实现网络接口
 * @version 0.1
 * @date 2024-05-23
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "netif_pcap.h"

#include "dbg.h"
#include "exmsg.h"
#include "pcap.h"
#include "sys_plat.h"
#include "ether.h"

/**
 * @brief 网络包接收线程
 *
 * @param arg
 */
void recv_thread(void *arg) {
  dbg_info(DBG_NETIF, "pcap recv thread is running....");
  // 获取到线程所管理的网络接口与pcap设备
  netif_t *netif = (netif_t *)arg;
  pcap_t *pcap = (pcap_t *)netif->ops_data;

  // 接收网络数据包
  while (1) {
    struct pcap_pkthdr *pkthdr =
        (struct pcap_pkthdr *)0;  // 记录数据包的信息，
                                   // 时间戳(ts)，总长度(len)，有效长度(caplen)
    const uint8_t *pktdata = (uint8_t *)0;  // 记录数据包内容的起始地址
    if (pcap_next_ex(pcap, &pkthdr, &pktdata) != 1) {  // 从pcap设备接收数据包
      continue;
    }

    // 接收到数据包，将数据包内容拷贝到pktbuf_t数据包中
    // 从数据包池中分配一个数据包
    pktbuf_t *pktbuf = pktbuf_alloc(pkthdr->len);
    if (pktbuf == (pktbuf_t *)0) {  // 失败，丢弃从pcap设备接收到的数据包
      dbg_warning(DBG_NETIF, "packet loss: pktbuf == NULL!");
      continue;
    }

    // pktbuf分配成功，将数据包内容拷贝到pktbuf中
    net_err_t err = pktbuf_write(pktbuf, (uint8_t *)pktdata, pkthdr->len);
    if (err != NET_ERR_OK) {  // 失败，释放pktbuf
      pktbuf_free(pktbuf);
      dbg_warning(DBG_NETIF, "packet loss: pktbuf write failed!");
      continue;
    }

    // 数据包拷贝成功，将数据包放入接收队列
    err = netif_recvq_put(netif, pktbuf, 0);  // 阻塞方式放入接收队列
    if (err != NET_ERR_OK) {                  // 失败，释放pktbuf
      pktbuf_free(pktbuf);
      dbg_warning(DBG_NETIF, "packet loss: netif recvq put failed!");
      continue;
    }

    // TODO: 数据包已放入接收队列，不由当前线程管理，不需要释放pktbuf
  }
}

/**
 * @brief 网络包发送线程
 *
 * @param arg
 */
void send_thread(void *arg) {
  dbg_info(DBG_NETIF, "send thread is running.");

  // 数据包发送缓冲区
  static uint8_t send_buf[NET_MAC_FRAME_MAX_SIZE];

  // 获取到线程所管理的网络接口与pcap设备
  netif_t *netif = (netif_t *)arg;
  pcap_t *pcap = (pcap_t *)netif->ops_data;

  // 发送网络数据包
  while (1) {
    pktbuf_t *buf =
        netif_sendq_get(netif, 0);  // 阻塞方式获取发送队列中的数据包
    if (buf == (pktbuf_t *)0) {
      continue;
    }

    int total_size = pktbuf_total_size(buf);  // 获取数据包的总长度
    pktbuf_read(buf, send_buf, total_size);  // 读取数据包内容

    pktbuf_free(buf);  // TODO: 当前线程成功完成数据包的最后处理，释放该数据包

    if (pcap_inject(pcap, send_buf, total_size) == -1) {  // 发送数据包
      dbg_warning(DBG_NETIF, "pcap send packet failed: %s", pcap_geterr(pcap));
      dbg_warning(DBG_NETIF, "packet size: %d", total_size);
    }
  }
}

/**
 * @brief 打开pcap接口
 * pcap接口是一个模拟的网络接口，使用pcap库模拟网卡驱动
 * 通过pcap库可以将网络数据包发送到网络上，也可以从网络上接收数据包
 * 需要进行网络帧的封装和解封装，以及设置相应的网卡硬件参数
 *
 * @param netif
 * @param data
 * @return net_err_t
 */
static net_err_t netif_pcap_open(netif_t *netif, void *data) {
  // 获取打开pcap设备的参数
  pcap_data_t *pcap_data = (pcap_data_t *)data;

  // 打开pcap设备接口(使用ip地址和硬件地址打开本地主机对应的网卡)
  pcap_t *pcap = pcap_device_open(pcap_data->ip, pcap_data->hwaddr);
  if (pcap == (pcap_t *)0) {
    dbg_error(DBG_NETIF, "pcap open failed! netif name: %s", netif->name);
    return NET_ERR_DEV;
  }

  // 设置网络接口类型为以太网接口
  netif->type = NETIF_TYPE_ETHER;
  // 设置最大传输单元(ETHER II 帧格式中mtu为1500字节)
  netif->mtu = ETHER_MTU;
  // 记录打开的pcap设备
  netif->ops_data = pcap;
  // 设置接口硬件地址
  netif_set_hwaddr(netif, pcap_data->hwaddr, 6);  // MAC地址长度为6字节

  // 创建pcap接口的接收线程
  sys_thread_create(recv_thread, (void *)netif);
  // 创建pcap接口的发送线程
  sys_thread_create(send_thread, (void *)netif);

  return NET_ERR_OK;
}

/**
 * @brief 关闭pcap接口
 *
 * @param netif
 * @return net_err_t
 */
static void netif_pcap_close(netif_t *netif) {
  pcap_t *pcap = (pcap_t *)netif->ops_data;
  if (pcap) {
    pcap_close(pcap);
  }
}

/**
 * @brief 使用pcap接口发送数据
 *
 * @param netif
 * @param data
 * @param len
 * @return net_err_t
 */
static net_err_t netif_pcap_send(netif_t *netif) { return NET_ERR_OK; }

/**
 * @brief 环回接口的操作方法
 *
 */
const netif_ops_t netif_pcap_ops = {
    .open = netif_pcap_open,
    .close = netif_pcap_close,
    .send = netif_pcap_send,
};
