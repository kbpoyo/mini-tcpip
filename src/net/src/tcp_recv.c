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
#include "tools.h"

/**
 * @brief 初始化一个tcp数据包信息结构
 * 
 * @param tcp_info 
 * @param tcp_buf 
 * @param local_ip 
 * @param remote_ip 
 */
static void tcp_info_init(tcp_info_t *tcp_info, pktbuf_t *tcp_buf, ipaddr_t *local_ip, ipaddr_t *remote_ip) {
    tcp_info->tcp_buf = tcp_buf;
    tcp_info->tcp_hdr = (tcp_hdr_t *)pktbuf_data_ptr(tcp_buf);

    ipaddr_copy(&tcp_info->local_ip, local_ip);
    ipaddr_copy(&tcp_info->remote_ip, remote_ip);

    // 计算tcp数据包的有效数据长度
    tcp_info->data_len = pktbuf_total_size(tcp_buf) - tcp_hdr_size(tcp_info->tcp_hdr);
    
    // 记录tcp数据包的序列号和序列号长度
    tcp_info->seq = tcp_info->tcp_hdr->seq;

    /**
     * tcp协议为每一个有效数据字节都分配一个序列号
     * 并且也为SYN和FIN标志位各分配一个序列号
     * 拥有序列号的数据才是tcp确保可靠传输的数据
     * 序列号长度将用于接收方进行累积确认
     */
    tcp_info->seq_len = tcp_info->data_len + tcp_info->tcp_hdr->f_syn + tcp_info->tcp_hdr->f_fin;

}

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
  if (buf_size < sizeof(tcp_hdr_t) || buf_size < tcp_hdr_size(tcp_hdr)) {
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
  tcp_hdr->src_port = net_ntohs(tcp_hdr->src_port);
  tcp_hdr->dest_port = net_ntohs(tcp_hdr->dest_port);
  tcp_hdr->seq = net_ntohl(tcp_hdr->seq);
  tcp_hdr->ack = net_ntohl(tcp_hdr->ack);
  tcp_hdr->win_size = net_ntohs(tcp_hdr->win_size);
  tcp_hdr->checksum = net_ntohs(tcp_hdr->checksum);
  tcp_hdr->urg_ptr = net_ntohs(tcp_hdr->urg_ptr);

  // 记录tcp数据包信息
  tcp_info_t tcp_info;
  tcp_info_init(&tcp_info, tcp_buf, dest_ip, src_ip);
//   tcp_send_reset();

  return NET_ERR_OK;
}