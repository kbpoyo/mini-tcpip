/**
 * @file tcp_state.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief tcp传输协议状态机处理模块
 * @version 0.1
 * @date 2024-10-24
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "tcp_state.h"

#include "tcp_send.h"
#include "tcp_recv.h"

/**
 * @brief 返回tcp状态的字符串表示
 *
 * @param state
 * @return const char*
 */
const char *tcp_state_name(tcp_state_t state) {
  static const char *state_name[] = {
      [TCP_STATE_CLOSED] = "CLOSED",
      [TCP_STATE_LISTEN] = "LISTEN",
      [TCP_STATE_SYN_SENT] = "SYN_SENT",
      [TCP_STATE_SYN_RCVD] = "SYN_RCVD",
      [TCP_STATE_ESTABLISHED] = "ESTABLISHED",
      [TCP_STATE_FIN_WAIT_1] = "FIN_WAIT_1",
      [TCP_STATE_FIN_WAIT_2] = "FIN_WAIT_2",
      [TCP_STATE_CLOSING] = "CLOSING",
      [TCP_STATE_TIME_WAIT] = "TIME_WAIT",
      [TCP_STATE_CLOSE_WAIT] = "CLOSE_WAIT",
      [TCP_STATE_LAST_ACK] = "LAST_ACK",
      [TCP_STATE_MAX] = "UNKNOWN",
  };

  if (state > TCP_STATE_MAX) {
    state = TCP_STATE_MAX;
  }

  return state_name[state];
}

/**
 * @brief 设置tcp对象的状态
 *
 * @param tcp
 * @param state
 */
void tcp_state_set(tcp_t *tcp, tcp_state_t state) {
  tcp->state = state;
  tcp_disp("tcp set state.", tcp);
}

/**
 * @brief 根据tcp的标志位来处理数据包的ack确认
 *
 * @param tcp
 * @param info
 * @return net_err_t
 */
net_err_t tcp_ack_process(tcp_t *tcp, tcp_info_t *info) {
  // 获取tcp数据包头部
  tcp_hdr_t *tcp_hdr = info->tcp_hdr;

  // 若tcp对象的syn_send标志位有效, 则该ack为syn请求的ack确认
  if (tcp->flags.syn_send) {
    tcp->send.una++; // 对端已确认接收syn号，更新未确认的序号
    tcp->flags.syn_send = 0;  // 清除syn_send标志位
  }

  return NET_ERR_OK;
}

/***********************************************************************************************************
 *  tcp recv状态处理系列函数
 **********************************************************************************************************/

static net_err_t tcp_closed_recv(tcp_t *tcp, tcp_info_t *info) {
  return NET_ERR_OK;
}

static net_err_t tcp_listen_recv(tcp_t *tcp, tcp_info_t *info) {
  return NET_ERR_OK;
}

static net_err_t tcp_syn_sent_recv(tcp_t *tcp, tcp_info_t *info) {
  // 获取tcp数据包头部
  tcp_hdr_t *tcp_hdr = info->tcp_hdr;

  // 若ack有效, 则检查ack号是否合法
  if (tcp_hdr->f_ack) {
    if (tcp_seq_before_eq(tcp_hdr->ack, tcp->send.isn) ||
        tcp_seq_after(tcp_hdr->ack, tcp->send.nxt)) {
      // ack 落在待确认的数据段范围之外, 发送复位数据包通知对端
      dbg_error(DBG_TCP, "tcp ack error.");
      return tcp_send_reset(info);
    }
  }

  // 若rst有效, 则判断是否为复位请求
  if (tcp_hdr->f_rst) {
    if (!tcp_hdr->f_ack) {
      // 若复位请求无ack确认, 则忽略该复位请求
      return NET_ERR_OK;
    }

    // 若复位请求有ack确认, 则接收复位请求，舍弃当前连接
    tcp_abort_connect(tcp, NET_ERR_TCP_RST);
  }

  // 若syn有效, 则判断是否为连接请求
  if (tcp_hdr->f_syn) {
    // 设置接收窗口的初始序号, 待接收序号和对应标志
    tcp->recv.isn = tcp_hdr->seq;
    tcp->recv.nxt = tcp_hdr->seq + 1;
    tcp->flags.recv_win_valid = 1;

    if (tcp_hdr->f_ack) {
      // 完成三次握手,切换到ESTABLISHED状态，并唤醒等待在该连接事件上的任务
      tcp_ack_process(tcp, info);  // 处理连接请求的ack确认
      tcp_send_ack(tcp, info);     // 对该请求发送ack确认
      tcp_state_set(tcp, TCP_STATE_ESTABLISHED);
      sock_wakeup(&tcp->sock_base, SOCK_WAIT_CONN, NET_ERR_OK);
    } else {  // 若对方同时发送syn请求, 则不会有ack确认
              // 进入SYN_RCVD状态,
              // 并再次发送syn请求，并且此次请求将携带对对方请求的ack确认，以进行四次握手
              // TODO: 同时请求的概率分成小，暂时无法进行测试
      tcp_state_set(tcp, TCP_STATE_SYN_RCVD);
      tcp_send_syn(tcp);
    }
  }

  return NET_ERR_OK;
}
static net_err_t tcp_syn_rcvd_recv(tcp_t *tcp, tcp_info_t *info) {
  return NET_ERR_OK;
}

static net_err_t tcp_established_recv(tcp_t *tcp, tcp_info_t *info) {
  // 获取tcp数据包头部
  tcp_hdr_t *tcp_hdr = info->tcp_hdr;

  // 若复位请求有效, 则接收复位请求，舍弃当前连接
  if (tcp_hdr->f_rst) {
    dbg_warning(DBG_TCP, "tcp recv rst.");
    return tcp_abort_connect(tcp, NET_ERR_TCP_RST);
  }

  // 在连接建立后, 不应该再收到syn请求，若收到则可能出现异常
  // 发送复位数据包通知对端，并舍弃当前连接
  if (tcp_hdr->f_syn) {
    dbg_warning(DBG_TCP, "tcp recv syn.");
    tcp_send_reset(info);
    return tcp_abort_connect(tcp, NET_ERR_TCP_RST);
  }

  // 若ack有效, 则处理ack确认
  if (tcp_ack_process(tcp, info) != NET_ERR_OK) {
    dbg_error(DBG_TCP, "tcp ack process failed.");
    return NET_ERR_TCP;
  }


  // 处理tcp包的数据部分
  tcp_recv_data(tcp, info);

  // 若fin有效, 则进入CLOSE_WAIT状态
  if (tcp_hdr->f_fin) {
    tcp_state_set(tcp, TCP_STATE_CLOSE_WAIT);
  }

  return NET_ERR_OK;
}
static net_err_t tcp_fin_wait_1_recv(tcp_t *tcp, tcp_info_t *info) {
  return NET_ERR_OK;
}
static net_err_t tcp_fin_wait_2_recv(tcp_t *tcp, tcp_info_t *info) {
  return NET_ERR_OK;
}
static net_err_t tcp_closing_recv(tcp_t *tcp, tcp_info_t *info) {
  return NET_ERR_OK;
}
static net_err_t tcp_time_wait_recv(tcp_t *tcp, tcp_info_t *info) {
  return NET_ERR_OK;
}
static net_err_t tcp_close_wait_recv(tcp_t *tcp, tcp_info_t *info) {
  return NET_ERR_OK;
}
static net_err_t tcp_last_ack_recv(tcp_t *tcp, tcp_info_t *info) {
  return NET_ERR_OK;
}

/**
 * @brief tcp recv状态机处理函数
 *
 * @param tcp
 * @param info
 * @return net_err_t
 */
net_err_t tcp_state_handler_recv(tcp_t *tcp, tcp_info_t *info) {
  const static tcp_state_handler_t tcp_state_handler_recv[] = {
      [TCP_STATE_CLOSED] = tcp_closed_recv,
      [TCP_STATE_LISTEN] = tcp_listen_recv,
      [TCP_STATE_SYN_SENT] = tcp_syn_sent_recv,
      [TCP_STATE_SYN_RCVD] = tcp_syn_rcvd_recv,
      [TCP_STATE_ESTABLISHED] = tcp_established_recv,
      [TCP_STATE_FIN_WAIT_1] = tcp_fin_wait_1_recv,
      [TCP_STATE_FIN_WAIT_2] = tcp_fin_wait_2_recv,
      [TCP_STATE_CLOSING] = tcp_closing_recv,
      [TCP_STATE_TIME_WAIT] = tcp_time_wait_recv,
      [TCP_STATE_CLOSE_WAIT] = tcp_close_wait_recv,
      [TCP_STATE_LAST_ACK] = tcp_last_ack_recv,
  };

  if (!tcp || !info) {
    dbg_error(DBG_TCP, "tcp or info is null.");
    return NET_ERR_TCP;
  }

  if (tcp->state >= TCP_STATE_MAX || tcp->state < 0) {
    dbg_error(DBG_TCP, "tcp err or state error.");
    return NET_ERR_TCP;
  }

  return tcp_state_handler_recv[tcp->state](tcp, info);
}