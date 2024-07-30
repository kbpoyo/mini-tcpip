/**
 * @file ping.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 适配协议栈的ping程序
 * @version 0.1
 * @date 2024-07-29
 *
 * @copyright Copyright (c) 2024
 *
 */


#include <time.h>

#include "ping.h"
#include "net_api.h"
#include "sys_plat.h"

#define PING_DEFUALT_ID 0x0

static int ping_id = PING_DEFUALT_ID;

/**
 * @brief 计算ping数据包(icmp包)校验和
 *
 * @param data
 * @param len
 * @return uint16_t
 */
uint16_t ping_checksum16(const void *data, uint16_t len) {
  const uint16_t *data_ptr = (const uint16_t *)data;
  uint32_t checksum = 0;

  while (len > 1) {  // 每次读取两个字节即16位
    checksum += *(data_ptr++);
    len -= 2;
  }

  if (len) {  // 如果数据长度为奇数，最后一个字节单独处理
    checksum += *(uint8_t *)data_ptr;
  }

  uint16_t high = 0;
  // 校验和最终大小为16位， 如果校验和有进位，将进位加到低16位
  // 直到高16位为0
  while ((high = checksum >> 16) != 0) {
    checksum = high + (checksum & 0xffff);
  }

  return (uint16_t)~checksum;
}

/**
 * @brief 运行ping任务
 *
 * @param ping
 * @param dest_ip
 * @param data_size
 * @param count
 * @param interval
 */
void ping_run(ping_t *ping, const char *dest_ip, int data_size,
              int repeat_count, int interval) {
  // 初始化WinSock
  // WSADATA wsaData;
  // WSAStartup(MAKEWORD(2, 2), &wsaData);

  // 在本地创建raw_socket
  SOCKET raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (raw_socket < 0) {
    plat_printf("create raw socket error\n");
    return;
  }

  // 设置socket属性
  int tmo = 3000;  // 超时时间3s
#if 0
  struct timeval tmo = {tmo / 1000, (tmo % 1000) * 1000};
#endif
  setsockopt(raw_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tmo,
             sizeof(tmo));

  // 初始化远端socket地址信息
  struct sockaddr_in dest_addr;
  plat_memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_addr.s_addr = inet_addr(dest_ip);
  dest_addr.sin_port = 0;

  // 创建指定的socket连接
  if (connect(raw_socket, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
    plat_printf("connect error\n");
    closesocket(raw_socket);
    return;
  }

  // 填充ping数据包
  int fill_size = data_size > PING_BUF_SIZE ? PING_BUF_SIZE : data_size;
  for (int i = 0; i < fill_size; i++) {
    ping->req.buf[i] = i;
  }

  // 发送ping数据包
  for (int i = 0; i < repeat_count; i++) {
    // 填充icmp头部字段
    ping->req.icmp_hdr.type = 8;
    ping->req.icmp_hdr.code = 0;  // type:8 code:0表示icmp回显请求
    ping->req.icmp_hdr.chksum = 0;
    ping->req.icmp_hdr.id = ping_id;
    ping->req.icmp_hdr.seq = i;

    // 计算icmp校验和
    ping->req.icmp_hdr.chksum =
        ping_checksum16(&ping->req, sizeof(icmp_hdr_t) + fill_size);

// 发送icmp数据包
#if 0
    int size = sendto(raw_socket, (const char *)&ping->req,
                      sizeof(icmp_hdr_t) + fill_size, 0,
                      (struct sockaddr *)&dest_addr, sizeof(dest_addr));
#endif
    int size = send(raw_socket, (const char *)&ping->req,
                    sizeof(icmp_hdr_t) + fill_size, 0);

    if (size < 0) {  // 发送失败
      plat_printf("send icmp packet error\n");
      break;
    }

    // 获取发送时间
    clock_t send_time = clock();

    do {
      // 接收ping应答
      plat_memset(&ping->reply, 0, sizeof(ping->reply));
      struct sockaddr_in src_addr;
      int addr_len = sizeof(src_addr);
#if 0
      size = recvfrom(raw_socket, (char *)&ping->reply, sizeof(ping->reply), 0,
                      (struct sockaddr *)&src_addr, &addr_len);
#endif
      size = recv(raw_socket, (char *)&ping->reply, sizeof(ping->reply), 0);
      if (size < 0) {
        plat_printf("recv icmp packet tmo\n");
        break;
      }

      // 判断该应答是否是对应的请求
      if (ping->reply.icmp_hdr.id == ping->req.icmp_hdr.id &&
          ping->reply.icmp_hdr.seq == ping->req.icmp_hdr.seq) {  // 是对应的请求
        break;
      }
    } while (1);

    // 接收响应成功
    if (size >= 0) {
      // 原始套接字(raw_socket)会直接向应用程序递交ip数据包，所以需要减去ip头部和icmp头部的长度
      int recv_size = size - sizeof(ip_hdr_t) - sizeof(icmp_hdr_t);
      if (plat_memcmp(ping->req.buf, ping->reply.buf, recv_size) != 0) {
        plat_printf("ping %s, seq = %d, recv_size = %d, fail\n", dest_ip, i,
                    recv_size);
        continue;
      }

      if (recv_size == fill_size) {
        plat_printf("ping %s, seq = %d, bytes = %d, ", dest_ip, i, recv_size);
      } else {
        plat_printf("ping %s, seq = %d, bytes = %d(send = %d), ", dest_ip, i,
                    recv_size);
      }

      // 计算延迟
      int diff_ms = (clock() - send_time) * 1000 / CLOCKS_PER_SEC;
      if (diff_ms < 1) {
        plat_printf("time < 1ms, TTL = %d\n", ping->reply.ip_hdr.ttl);
      } else {
        plat_printf("time = %dms, TTL = %d\n", diff_ms, ping->reply.ip_hdr.ttl);
      }

      // 等待interval时间
      sys_sleep(interval);
    }
  }

  // 关闭WinSock
  closesocket(raw_socket);
}
