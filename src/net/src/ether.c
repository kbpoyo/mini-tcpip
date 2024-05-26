/**
 * @file ether.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 链路层：以太网协议层接口实现
 * @version 0.1
 * @date 2024-05-26
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "ether.h"
#include "netif.h"
 

static net_err_t ether_frame_check(ether_frame_t *frame, int total_size) {
    if (total_size > sizeof(ether_hdr_t) + ETHER_MTU) { // 以太网帧大小超过最大值
        dbg_warning(DBG_ETHER, "ether frame size is too large.");
        return NET_ERR_SIZE;
    }

    //! 以太网帧最小值应该为 MAC包头(14B) + 最小负载(46B) = 60B
    //! 但是PCAP库会自动将有效载荷中的填充字节去掉，所以导致接收到的以太网帧的有效载荷
    //! 可能小于46B，所以这里只判断以太网帧的大小是否小于MAC包头的大小， 不对有效载荷大小进行判断
    if (total_size < sizeof(ether_hdr_t)) { // 以太网帧大小小于最小值
        dbg_warning(DBG_ETHER, "ether frame size is too small.");
        return NET_ERR_SIZE;
    }
  return NET_ERR_OK;
}

/**
 * @brief 进行以太网协议层相关的初始化
 *
 * @param netif
 * @return net_err_t
 */
static net_err_t ether_open(netif_t *netif) { return NET_ERR_OK; }

/**
 * @brief 完成以太网协议层相关的清理
 *
 * @param netif
 */
static void ether_close(netif_t *netif) { return; }

/**
 * @brief 从以太网协议层接收数据包
 *
 * @param netif
 * @param pktbuf
 * @return net_err_t
 */
static net_err_t ether_recv(netif_t *netif, pktbuf_t *buf) {

    dbg_info(DBG_ETHER, "link layer ether recving....");

    net_err_t err = NET_ERR_OK;

    // 获取以太网帧
    ether_frame_t *frame = (ether_frame_t *)pktbuf_data_ptr(buf);

    if ((err = ether_frame_check(frame, pktbuf_total_size(buf))) != NET_ERR_OK) {
        dbg_error(DBG_ETHER, "link layer ether recv failed.");
        return err;
    }

    // 数据包处理成功，由当前函数释放数据包
    pktbuf_free(buf);

    dbg_info(DBG_ETHER, "link layer ether recv ok.");

  return NET_ERR_OK;
}

/**
 * @brief 向以太网协议层发送数据包
 *
 * @param netif
 * @param ipdest
 * @param buf
 * @return net_err_t
 */
static net_err_t ether_send(netif_t *netif, ipaddr_t *ipdest, pktbuf_t *buf) {
  return NET_ERR_OK;
}

/**
 * @brief 初始化以太网协议层
 *
 * @return net_err_t
 */
net_err_t ether_module_init(void) {
  static const link_layer_t ether_ops = {
      // 将接口暴露给外部
      .type = NETIF_TYPE_ETHER, .open = ether_open, .close = ether_close,
      .recv = ether_recv,       .send = ether_send,
  };

  dbg_info(DBG_ETHER, "init ether....");

  // 将以太网协议层回调接口注册到网络接口模块的链路层回调接口中
  net_err_t err = netif_layer_register(&ether_ops);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_ETHER, "register ether failed.");
    return err;
  }

  dbg_info(DBG_ETHER, "init ether ok.");
}
