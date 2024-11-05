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

#include "tcp_recv.h"
#include "tcp_send.h"

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
 * @brief 处理对端发来的ack确认，以更新本地tcp对象标志位以及发送窗口
 *
 * @param tcp
 * @param info
 * @return net_err_t
 */
net_err_t tcp_ack_process(tcp_t *tcp, tcp_info_t *info) {
  // 获取tcp数据包头部
  tcp_hdr_t *tcp_hdr = info->tcp_hdr;

  // 若tcp对象的syn_send标志位有效, 则该ack一定为syn请求的ack确认
  if (tcp->flags.syn_send) {
    tcp->send.una++;  // 对端已确认接收syn号，更新未确认的序号
    tcp->flags.syn_send = 0;  // 清除syn_send标志位
  }

  // 若tcp对象的fin_send标志位有效, 则判断该ack是否为fin请求的ack确认
  if (tcp->flags.fin_send && tcp_seq_before(tcp->send.una, tcp_hdr->ack)) {
    // 当前ack序号大于发送窗口的未确认序号，说明对端已对本地的fin请求进行了确认
    tcp->send.una++;  // 对端已确认接收fin号，更新未确认的序号
    tcp->flags.fin_send = 0;  // 清除fin_send标志位
  }

  return NET_ERR_OK;
}

/**
 * @brief 使用定时器来将tcp进入TIME_WAIT状态
 *
 */
void tcp_state_time_wait(tcp_t *tcp) {
  tcp_state_set(tcp, TCP_STATE_TIME_WAIT);
}

/***********************************************************************************************************
 *  tcp recv状态处理系列函数
 **********************************************************************************************************/

static net_err_t tcp_closed_recv(tcp_t *tcp, tcp_info_t *info) {
  // 本地tcp对象因某些错误被tcp_abort_connect()函数设置为CLOSED状态，
  // 即已经断开连接，只是持有该资源的用户线程还未调用close()进行资源释放
  // 所以不需要接收任何数据，直接向对端发送复位数据包即可，若接收到的包为复位请求，则忽略即可
  if (!info->tcp_hdr->f_rst) {
    tcp_send_reset(info);
  }

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
    dbg_warning(DBG_TCP, "tcp recv rst in ESTABLISHED.");
    return tcp_abort_connect(tcp, NET_ERR_TCP_RST);
  }

  // 在连接建立后, 不应该再收到syn请求，若收到则可能出现异常
  // 发送复位数据包通知对端，并舍弃当前连接
  if (tcp_hdr->f_syn) {
    dbg_warning(DBG_TCP, "tcp recv syn in ESTABLISHED.");
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

  // 若fin有效, 则对端请求关闭，本地进入CLOSE_WAIT状态
  if (tcp_hdr->f_fin) {
    tcp_state_set(tcp, TCP_STATE_CLOSE_WAIT);
  }

  return NET_ERR_OK;
}

static net_err_t tcp_fin_wait_1_recv(tcp_t *tcp, tcp_info_t *info) {
  // 获取tcp数据包头部
  tcp_hdr_t *tcp_hdr = info->tcp_hdr;

  // 若复位请求有效, 则接收复位请求，舍弃当前连接
  if (tcp_hdr->f_rst) {
    dbg_warning(DBG_TCP, "tcp recv rst in FIN_WAIT_1.");
    return tcp_abort_connect(tcp, NET_ERR_TCP_RST);
  }

  // 在连接建立后, 不应该再收到syn请求，若收到则可能出现异常
  // 发送复位数据包通知对端，并舍弃当前连接
  if (tcp_hdr->f_syn) {
    dbg_warning(DBG_TCP, "tcp recv syn in FIN_WAIT_1.");
    tcp_send_reset(info);
    return tcp_abort_connect(tcp, NET_ERR_TCP_RST);
  }

  // 若ack有效, 则处理ack确认(更新发送窗口信息)
  if (tcp_ack_process(tcp, info) != NET_ERR_OK) {
    dbg_error(DBG_TCP, "tcp ack process failed in FIN_WAIT_1.");
    return NET_ERR_TCP;
  }

  // 处理tcp包的数据部分(更新接收窗口信息, 并发送ack确认)
  tcp_recv_data(tcp, info);

  // 根据对端是否也请求关闭连接，进入不同的状态
  if (tcp->flags.fin_send == 0) {  // 对端已确认本地的fin请求
    if (tcp_hdr->f_fin) {
      // 对端在确认本地fin请求的同时也发送了fin(在数据处理中已对其进行确认),
      // 本地直接进入TIME_WAIT状态
      tcp_state_time_wait(tcp);
    } else {
      // 进入FIN_WAIT_2状态，以等待对端发送的fin请求
      tcp_state_set(tcp, TCP_STATE_FIN_WAIT_2);
    }
  } else if (tcp_hdr->f_fin) {
    // 对端未确认本地fin请求，但却发送了fin请求，本地进入CLOSING状态
    // 以等待对端对本地的fin请求进行确认
    // TODO: 暂时无法测试双方同时发送fin请求的情况
    tcp_state_set(tcp, TCP_STATE_CLOSING);
  }

  return NET_ERR_OK;
}
static net_err_t tcp_fin_wait_2_recv(tcp_t *tcp, tcp_info_t *info) {
  // 获取tcp数据包头部
  tcp_hdr_t *tcp_hdr = info->tcp_hdr;

  // 若复位请求有效, 则接收复位请求，舍弃当前连接
  if (tcp_hdr->f_rst) {
    dbg_warning(DBG_TCP, "tcp recv rst in FIN_WAIT_2.");
    return tcp_abort_connect(tcp, NET_ERR_TCP_RST);
  }

  // 在连接建立后, 不应该再收到syn请求，若收到则可能出现异常
  // 发送复位数据包通知对端，并舍弃当前连接
  if (tcp_hdr->f_syn) {
    dbg_warning(DBG_TCP, "tcp recv syn in FIN_WAIT_2.");
    tcp_send_reset(info);
    return tcp_abort_connect(tcp, NET_ERR_TCP_RST);
  }

  // 若ack有效, 则处理ack确认(更新发送窗口信息)
  if (tcp_ack_process(tcp, info) != NET_ERR_OK) {
    dbg_error(DBG_TCP, "tcp ack process failed in FIN_WAIT_2.");
    return NET_ERR_TCP;
  }

  // 处理tcp包的数据部分(更新接收窗口信息, 并发送ack确认)
  tcp_recv_data(tcp, info);

  // 若对端请求关闭连接，本地进入TIME_WAIT状态
  if (tcp_hdr->f_fin) {
    tcp_state_time_wait(tcp);
  }

  return NET_ERR_OK;
}
static net_err_t tcp_closing_recv(tcp_t *tcp, tcp_info_t *info) {
  // 获取tcp数据包头部
  tcp_hdr_t *tcp_hdr = info->tcp_hdr;

  // 若复位请求有效, 则接收复位请求，舍弃当前连接
  if (tcp_hdr->f_rst) {
    dbg_warning(DBG_TCP, "tcp recv rst in CLOSING.");
    return tcp_abort_connect(tcp, NET_ERR_TCP_RST);
  }

  // 在连接建立后, 不应该再收到syn请求，若收到则可能出现异常
  // 发送复位数据包通知对端，并舍弃当前连接
  if (tcp_hdr->f_syn) {
    dbg_warning(DBG_TCP, "tcp recv syn in CLOSING.");
    tcp_send_reset(info);
    return tcp_abort_connect(tcp, NET_ERR_TCP_RST);
  }

  // 若ack有效, 则处理ack确认(更新发送窗口信息)
  if (tcp_ack_process(tcp, info) != NET_ERR_OK) {
    dbg_error(DBG_TCP, "tcp ack process failed in CLOSING.");
    return NET_ERR_TCP;
  }

  // 双方同时发送fin请求的情况，不需要再进行有效数据的接收处理(之前已完成全部接收和确认)
  // 只需要接收对端对本地fin请求的ack确认，以及发送对端fin请求的ack确认(已在ack处理中完成)
  if (tcp->flags.fin_send == 0) {
    // 对端已确认本地的fin请求，本地进入TIME_WAIT状态
    tcp_state_time_wait(tcp);
  }

  return NET_ERR_OK;
}

static net_err_t tcp_time_wait_recv(tcp_t *tcp, tcp_info_t *info) {
  // 获取tcp数据包头部
  tcp_hdr_t *tcp_hdr = info->tcp_hdr;

  // 若复位请求有效, 则接收复位请求，舍弃当前连接
  if (tcp_hdr->f_rst) {
    dbg_warning(DBG_TCP, "tcp recv rst in TIME_WAIT.");
    return tcp_abort_connect(tcp, NET_ERR_TCP_RST);
  }

  // 在连接建立后, 不应该再收到syn请求，若收到则可能出现异常
  // 发送复位数据包通知对端，并舍弃当前连接
  if (tcp_hdr->f_syn) {
    dbg_warning(DBG_TCP, "tcp recv syn in TIME_WAIT.");
    tcp_send_reset(info);
    return tcp_abort_connect(tcp, NET_ERR_TCP_RST);
  }

  // 若ack有效, 则处理ack确认(更新发送窗口信息)
  if (tcp_ack_process(tcp, info) != NET_ERR_OK) {
    dbg_error(DBG_TCP, "tcp ack process failed in TIME_WAIT.");
    return NET_ERR_TCP;
  }

  // 在TIME_WAIT状态下，双方都已关闭连接，不需要再进行有效数据的接收处理(之前已完成全部接收和确认，包括对端发送的fin请求)
  // 只需要单独确认对端重传的fin请求(在之前ack丢失的情况下)
  if (tcp_hdr->f_fin) {
    // TODO: 暂时无法测试ack丢失的情况
    tcp_send_ack(tcp, info);  // 发送ack确认
    tcp_state_time_wait(
        tcp);  // 重新进入TIME_WAIT状态(重置定时器，继续等待2MSL时长，以确保对端已经接收到ack确认)
  }

  return NET_ERR_OK;
}

static net_err_t tcp_close_wait_recv(tcp_t *tcp, tcp_info_t *info) {
  // 获取tcp数据包头部
  tcp_hdr_t *tcp_hdr = info->tcp_hdr;

  // 若复位请求有效, 则接收复位请求，舍弃当前连接
  if (tcp_hdr->f_rst) {
    dbg_warning(DBG_TCP, "tcp recv rst in TIME_WAIT.");
    return tcp_abort_connect(tcp, NET_ERR_TCP_RST);
  }

  // 在连接建立后, 不应该再收到syn请求，若收到则可能出现异常
  // 发送复位数据包通知对端，并舍弃当前连接
  if (tcp_hdr->f_syn) {
    dbg_warning(DBG_TCP, "tcp recv syn in TIME_WAIT.");
    tcp_send_reset(info);
    return tcp_abort_connect(tcp, NET_ERR_TCP_RST);
  }

  // 若ack有效, 则处理ack确认(更新发送窗口信息)
  if (tcp_ack_process(tcp, info) != NET_ERR_OK) {
    dbg_error(DBG_TCP, "tcp ack process failed in TIME_WAIT.");
    return NET_ERR_TCP;
  }

  // 在CLOSE_WAIT状态下，对端已经关闭连接，不需要再进行有效数据的接收处理(之前已完成全部接收和确认)
  // 只需要对对端的fin请求进行重复的ack确认(在之前ack丢失的情况下)

  return NET_ERR_OK;
}

static net_err_t tcp_last_ack_recv(tcp_t *tcp, tcp_info_t *info) {
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

  // 对端已关闭连接，不需要再进行有效数据的接收处理(之前已完成全部接收和确认)
  //  且本地已经发送了fin请求，只需要接收对端对fin请求的确认，
  //  并且对对端的fin请求进行重复的ack确认(在之前ack丢失的情况下) 即可。
  if (tcp->flags.fin_send == 0) {
    tcp_abort_connect(tcp, NET_ERR_TCP_CLOSE);
  }

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