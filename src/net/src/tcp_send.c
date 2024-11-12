/**
 * @file tcp_send.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief tcp传输协议模块发送数据的相关处理方法
 * @version 0.1
 * @date 2024-10-12
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "tcp_send.h"

#include "protocol.h"
#include "tcp_buf.h"
#include "tools.h"

/**
 * @brief tcp协议层将tcp数据包下交给网络层处理
 *
 * @param tcp_hdr
 * @param tcp_buf
 * @param dest_ip
 * @param src_ip
 * @return net_err_t
 */
static net_err_t tcp_send(tcp_hdr_t *tcp_hdr, pktbuf_t *tcp_buf,
                          ipaddr_t *dest_ip, ipaddr_t *src_ip) {
  tcp_disp_pkt("tcp send pkt.", tcp_hdr, tcp_buf);
  // 将tcp头部字段转换为网络字节序
  tcp_hdr_hton(tcp_hdr);

  // 清空校验和字段, 并计算校验和
  tcp_hdr->checksum = 0;
  tcp_hdr->checksum =
      tools_checksum16_pseudo_head(tcp_buf, dest_ip, src_ip, NET_PROTOCOL_TCP);

  // 通过网络层发送tcp数据包
  net_err_t err = ipv4_send(NET_PROTOCOL_TCP, dest_ip, src_ip, tcp_buf);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_TCP, "tcp packet send err.");
  }

  return err;
}

/**
 * @brief 根据接收到的tcp包的信息(info),向发送方发送一个tcp复位数据包
 *
 * @param info
 * @return net_err_t
 */
net_err_t tcp_send_reset(tcp_info_t *info) {
  // 分配一个数据包用于存放tcp复位数据包
  pktbuf_t *buf = pktbuf_alloc(sizeof(tcp_hdr_t));  //!!! 分配数据包
  if (!buf) {
    dbg_warning(DBG_TCP, "no free pktbuf for tcp reset pkt.");
    return NET_ERR_TCP;
  }

  // 获取tcp数据包头部, 并填充头部字段
  tcp_hdr_t *tcp_hdr = (tcp_hdr_t *)pktbuf_data_ptr(buf);
  plat_memset(tcp_hdr, 0, sizeof(tcp_hdr_t));
  tcp_hdr->src_port = info->tcp_hdr->dest_port;
  tcp_hdr->dest_port = info->tcp_hdr->src_port;
  tcp_set_hdr_size(tcp_hdr, sizeof(tcp_hdr_t));
  tcp_hdr->reserved = 0;
  tcp_hdr->flag = 0;   // 清空标志位
  tcp_hdr->f_rst = 1;  // 设置复位标志位
  tcp_hdr->win_size = tcp_hdr->urg_ptr = 0;

  /**
   * @brief 根据对方发送的ack号，更新发送的seq号和ack号
   *        若对端收到正确的ack号，则对端会立即响应复位请求，在0.5s后尝试重传，共重传4次(每次间隔0.5s)
   *        若没有设置ack号，则对端也会重传4次，但第一次间隔1s，后面每次间隔时间翻倍
   *        若对端收到错误的ack号，则对端会发送一个复位数据包，以关闭连接
   * 
   */
  if (info->tcp_hdr->f_ack) {
    tcp_hdr->seq = info->tcp_hdr->ack;  // 根据对方发送的ack号，更新发送的seq号
    tcp_hdr->ack = 0;
    tcp_hdr->f_ack = 0;
  } else {
    tcp_hdr->seq = 0;
    tcp_hdr->ack = info->tcp_hdr->seq +
                   info->seq_len;  // 根据对方发送的seq号，更新发送的ack号
    tcp_hdr->f_ack = 1;
  }

  // 发送tcp复位数据包
  net_err_t err = tcp_send(tcp_hdr, buf, &info->remote_ip, &info->local_ip);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_TCP, "tcp send reset failed.");
    pktbuf_free(buf);  //!!! 释放数据包
  }

  return err;
}

/**
 * @brief 将tcp发送缓冲区中的数据拷贝到待发送的tcp数据包中
 *        从缓冲区out指针偏移offset字节处开始读取数据
 *
 * @param tcp
 * @param buf
 * @return int 拷贝数据的长度
 */
static int copy_send_data(tcp_t *tcp, pktbuf_t *buf) {
  // 获取发送缓冲区中待发送数据的长度, 数据长度不能超过mss
  int valid_len = tcp_wait_send_data(tcp);
  int cpy_len = MIN(valid_len, tcp->mss);
  if (cpy_len <= 0) {
    return cpy_len;
  }

  // 对数据包进行扩容，以容纳新的数据
  net_err_t err = pktbuf_resize(buf, pktbuf_total_size(buf) + cpy_len);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_TCP, "pktbuf resize failed.");
    return -1;
  }

  // 移动数据包的访问指针，以略过tcp头部
  pktbuf_acc_reset(buf);
  pktbuf_seek(buf, tcp_get_hdr_size((tcp_hdr_t *)pktbuf_data_ptr(buf)));

  // 从发送缓冲区中读取数据到tcp数据包中
  return tcp_buf_read_to_pktbuf(&tcp->send.buf, buf, tcp_wait_ack_data(tcp),
                                cpy_len);  //!!! 数据包传递
}

/**
 * @brief 根据tcp对象的状态，创建并发送一个tcp数据包
 *
 * @param tcp
 * @return net_err_t
 */
net_err_t tcp_transmit(tcp_t *tcp) {
  net_err_t err = NET_ERR_OK;

  // 获取tcp对象的发送缓冲区中待发送数据的长度，并判断是否需要发送一个tcp数据包
  int wait_data_len = tcp_wait_send_data(tcp);
  if (wait_data_len < 0) {
    dbg_error(DBG_TCP, "tcp send buf error, wait_send_data can't < 0.");
    return NET_ERR_TCP;
  }
  int seq_len =
      tcp->flags.syn_need_send + tcp->flags.fin_need_send + wait_data_len;
  if (seq_len == 0) {  // 没有数据或请求需要发送，直接返回
    return NET_ERR_OK;
  }

  // 分配一个数据包用于存放tcp数据包
  pktbuf_t *buf = pktbuf_alloc(sizeof(tcp_hdr_t));  //!!! 分配数据包
  if (!buf) {
    dbg_warning(DBG_TCP, "no free pktbuf for tcp pkt.");
    return NET_ERR_TCP;
  }

  // 获取tcp数据包头部, 并填充头部字段
  tcp_hdr_t *tcp_hdr = (tcp_hdr_t *)pktbuf_data_ptr(buf);
  tcp_hdr->src_port = tcp->sock_base.local_port;
  tcp_hdr->dest_port = tcp->sock_base.remote_port;
  tcp_hdr->seq = tcp->send.nxt;  // 设置序号位发送窗口的下一个待发送数据段序号
  tcp_hdr->ack = tcp->recv.nxt;  // 设置确认号为接收窗口的下一个待接收数据段序号
  tcp_set_hdr_size(tcp_hdr, sizeof(tcp_hdr_t));  // 设置头部长度
  tcp_hdr->reserved = 0;                         // 清空保留字段
  tcp_hdr->flag = 0;                             // 清空标志位
  // 根据tcp标志的syn_need_send位来设置SYN标志位，以请求连接
  tcp_hdr->f_syn = tcp->flags.syn_need_send;
  // 根据tcp标志的fin_need_send标志位来设置FIN标志位，以请求关闭连接,
  // 且发送缓冲区中无待发送数据时，才能发送fin请求
  tcp_hdr->f_fin = (wait_data_len == 0 ? tcp->flags.fin_need_send : 0);
  // 根据tcp标志的recv_win_valid标志位来设置ACK标志位，确认已收到的tcp数据包
  tcp_hdr->f_ack = tcp->flags.recv_win_valid;
  tcp_hdr->win_size = 1024;  // 设置窗口大小
  tcp_hdr->urg_ptr = 0;      // 紧急指针

  // 记录tcp头部syn或fin标志位是否设置成功
  int temp_syn = tcp_hdr->f_syn;
  int temp_fin = tcp_hdr->f_fin;

  // 从发送缓冲区中读取数据到tcp数据包中,
  // 并通过tcp_send将tcp数据包下交给网络层处理
  int data_len = copy_send_data(tcp, buf);  //!!! 数据包传递
  if (data_len < 0) {
    goto tcp_transmit_failed;
  }
  err = tcp_send(tcp_hdr, buf, &tcp->sock_base.remote_ip,
                 &tcp->sock_base.local_ip);  //!!! 数据包传递
  if (err != NET_ERR_OK) {
    goto tcp_transmit_failed;
  }

  // tcp数据包发送成功，更新发送窗口信息(syn号和fin号都需要占用一个序号位)和标志位, 
  tcp->send.nxt += (temp_syn + temp_fin + data_len);  
  if (temp_syn) {
    tcp->flags.syn_need_send = 0;
    tcp->flags.syn_need_ack = 1;
  }
  if (temp_fin) {
    tcp->flags.fin_need_send = 0;
    tcp->flags.fin_need_ack = 1;
  }
  return NET_ERR_OK;

tcp_transmit_failed:
  dbg_error(DBG_TCP, "tcp transmit failed.");
  pktbuf_free(buf);  //!!! 释放数据包
  return err == NET_ERR_OK ? NET_ERR_TCP : err;
}

/**
 * @brief 单独对一个数据包发送ack确认
 *
 * @param tcp
 * @param info
 * @return net_err_t
 */
net_err_t tcp_send_ack(tcp_t *tcp, tcp_info_t *info) {
  pktbuf_t *buf = pktbuf_alloc(sizeof(tcp_hdr_t));  //!!! 分配数据包
  if (!buf) {
    dbg_warning(DBG_TCP, "no free pktbuf for tcp ack pkt.");
    return NET_ERR_TCP;
  }

  // 获取tcp数据包头部, 并填充头部字段
  tcp_hdr_t *tcp_hdr = (tcp_hdr_t *)pktbuf_data_ptr(buf);
  tcp_hdr->src_port = tcp->sock_base.local_port;
  tcp_hdr->dest_port = tcp->sock_base.remote_port;
  tcp_hdr->seq = tcp->send.nxt;  // 设置序号位发送窗口的下一个待发送数据段序号
  tcp_hdr->ack = tcp->recv.nxt;  // 设置确认号为接收窗口的下一个待接收数据段序号
  tcp_set_hdr_size(tcp_hdr, sizeof(tcp_hdr_t));  // 设置头部长度
  tcp_hdr->reserved = 0;                         // 清空保留字段
  tcp_hdr->flag = 0;                             // 清空标志位
  tcp_hdr->f_ack = 1;
  tcp_hdr->win_size = 1024;  // 设置窗口大小
  tcp_hdr->urg_ptr = 0;      // 紧急指针

  // 将tcp数据包下交给网络层处理
  net_err_t err = tcp_send(tcp_hdr, buf, &tcp->sock_base.remote_ip,
                           &tcp->sock_base.local_ip);  //!!! 数据包传递
  if (err != NET_ERR_OK) {
    dbg_error(DBG_TCP, "tcp send failed.");
    pktbuf_free(buf);  //!!! 释放数据包
  }

  return err;
}

/**
 * @brief 本地tcp对象向远端发送一个tcp连接请求(发送SYN标志位和初始序列号isn)
 *
 * @param tcp
 * @return net_err_t
 */
net_err_t tcp_send_syn(tcp_t *tcp) {
  // 设置tcp状态标志位
  tcp->flags.syn_need_send = 1;

  // 进行tcp发送数据包的相关处理
  return tcp_transmit(tcp);
}

/**
 * @brief 发送tcp连接关闭请求
 *
 * @param tcp
 * @return net_err_t
 */
net_err_t tcp_send_fin(tcp_t *tcp) {
  // 设置tcp标志位
  tcp->flags.fin_need_send = 1;

  // 进行tcp发送数据包的相关处理
  return tcp_transmit(tcp);
}

// int tcp_send_bufwrite(tcp_t *tcp, const uint8_t *buf, int len) {
//   // 获取发送缓冲区的剩余空间, 无空间则返回0
//   int free_cnt = tcp_buf_free_cnt(&tcp->send.buf);
//   if (free_cnt <= 0) {
//     return 0;
//   }

//   // 根据剩余空间大小，写入数据到发送缓冲区
//   int write_len = (len > free_cnt) ? free_cnt : len;
//   tcp_buf_write(&tcp->send.buf, buf, write_len);
//   return write_len;
// }
