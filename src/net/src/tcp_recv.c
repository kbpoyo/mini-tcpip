/**
 * @file tcp_recv.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief tcp协议模块接收数据的相关处理方法
 * @version 0.1
 * @date 2024-10-11
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "tcp_recv.h"

#include "ipaddr.h"
#include "pktbuf.h"
#include "protocol.h"
#include "tcp_send.h"
#include "tcp_state.h"
#include "tools.h"

/**
 * @brief 检查tcp数据包的合法性
 *
 * @param tcp_buf
 * @param src_ip
 * @param dest_ip
 * @return net_err_t
 */
static net_err_t tcp_check(pktbuf_t *tcp_buf, ipaddr_t *src_ip,
                           ipaddr_t *dest_ip) {
  // 获取tcp数据包头部和大小
  tcp_hdr_t *tcp_hdr = (tcp_hdr_t *)pktbuf_data_ptr(tcp_buf);
  int buf_size = pktbuf_total_size(tcp_buf);

  // 检查校验和是否正确
  if (tcp_hdr->checksum) {  // 校验和不为0，需要进行校验和计算
    uint16_t chksum = tools_checksum16_pseudo_head(tcp_buf, dest_ip, src_ip,
                                                   NET_PROTOCOL_TCP);
    if (chksum != 0) {
      dbg_error(DBG_TCP, "tcp checksum error.");
      return NET_ERR_TCP;
    }
  }

  // 检查tcp数据包大小是否正确
  if (buf_size < sizeof(tcp_hdr_t) || buf_size < tcp_get_hdr_size(tcp_hdr)) {
    dbg_error(DBG_TCP, "tcp packet size error.");
    return NET_ERR_TCP;
  }

  // 检查端口号是否合法
  if (!tcp_hdr->src_port || !tcp_hdr->dest_port) {
    dbg_error(DBG_TCP, "tcp port error.");
    return NET_ERR_TCP;
  }

  // 检查标志位是否有效
  if (!tcp_hdr->flag) {
    dbg_error(DBG_TCP, "tcp flag error.");
    return NET_ERR_TCP;
  }

  return NET_ERR_OK;
}

/**
 * @brief 从网络层接收tcp数据包，并进行处理
 *
 * @param buf
 * @param src_ip
 * @param dest_ip
 * @return net_err_t
 */
net_err_t tcp_recv(pktbuf_t *tcp_buf, ipaddr_t *src_ip, ipaddr_t *dest_ip) {
  // 检查tcp数据包的合法性
  net_err_t err = tcp_check(tcp_buf, src_ip, dest_ip);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_TCP, "tcp packet check failed.");
    return err;
  }

  // 获取tcp数据包头部, 并转换头部字段到主机字节序
  tcp_hdr_t *tcp_hdr = (tcp_hdr_t *)pktbuf_data_ptr(tcp_buf);
  tcp_hdr_ntoh(tcp_hdr);
  tcp_disp_pkt("recv tcp packet.", tcp_hdr, tcp_buf);

  // 记录tcp数据包信息
  tcp_info_t tcp_info;
  tcp_info_init(&tcp_info, tcp_buf, dest_ip, src_ip);

  // 根据tcp数据包信息查找对应的tcp对象
  tcp_t *tcp = tcp_find(&tcp_info);
  if (!tcp) {
    dbg_error(DBG_TCP, "tcp find failed.");
    // 发送一个tcp复位数据包
    tcp_send_reset(&tcp_info);
    tcp_disp_list();
    return NET_ERR_TCP;
  }

  // 根据tcp对象的状态处理接收到的tcp数据包
  err = tcp_state_handler_recv(tcp, &tcp_info);

  return NET_ERR_OK;
}

/**
 * @brief 接收处理tcp包的有效数据部分
 *
 * @param tcp
 * @param info
 * @return net_err_t
 */
net_err_t tcp_recv_data(tcp_t *tcp, tcp_info_t *info) {
  // 获取tcp数据包头部
  tcp_hdr_t *tcp_hdr = info->tcp_hdr;

  uint8_t wakeup = 0;  // 是否唤醒等待在tcp对象上的任务

  // 若fin有效, 则需要对fin位进行接收并确认
  if (tcp_hdr->f_fin) {
    tcp->recv.nxt++;
    wakeup++;
  }

  // TODO: 若数据包有效, 则需要对数据包进行接收并确认

  // 若成功处理完数据包，则唤醒等待在tcp对象上的任务
  if (wakeup) {
    if (tcp_hdr->f_fin) {
      // 对端请求关闭连接, 唤醒等待在tcp对象上的所有任务
      sock_wakeup(&tcp->sock_base, SOCK_WAIT_ALL, NET_ERR_TCP_CLOSE);
    } else {
      // 成功接收数据包, 唤醒等待在tcp对象上的读取任务
      sock_wakeup(&tcp->sock_base, SOCK_WAIT_READ, NET_ERR_OK);
    }

    // 发送ack确认
    tcp_send_ack(tcp, info);
  }

  return NET_ERR_OK;
}