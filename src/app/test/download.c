/**
 * @file download.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief tcp传输模块数据接收测试
 * @version 0.1
 * @date 2024-11-13
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <stdio.h>
#include <string.h>
// #include <winsock2.h>
#include "net_api.h"

#include "net_plat.h"

void download_test(const char *file_name, int port) {
  printf("download file: %s, from %s: %d\n", file_name, friend0_ip, port);

  int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd < 0) {
    printf("create socket error\n");
    return;
  }

  FILE *file = fopen(file_name, "wb");
  if (file == (FILE *)0) {
    printf("open file error\n");
    goto download_failed;
  }

  // 添加tcp保活机制
  int keep_alive = 1;
  int keep_idle = 60;
  int keep_interval = 5;
  int keep_count = 3;
  setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (char *)&keep_alive, sizeof(keep_alive)); // 开启keepalive属性
  setsockopt(sockfd, SOL_TCP, TCP_KEEPIDLE, (char *)&keep_idle, sizeof(keep_idle));
  setsockopt(sockfd, SOL_TCP, TCP_KEEPINTVL, (char *)&keep_interval, sizeof(keep_interval));
  setsockopt(sockfd, SOL_TCP, TCP_KEEPCNT, (char *)&keep_count, sizeof(keep_count));

  // 连接服务器
  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  servaddr.sin_addr.s_addr = inet_addr(friend0_ip);
  if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    printf("connect error\n");
    goto download_failed;
  }

  char buf[8192];
  int rcv_len = 0;
  ssize_t total_len = 0;
  while ((rcv_len = recv(sockfd, buf, sizeof(buf), 0)) > 0) {
    fwrite(buf, 1, rcv_len, file);
    fflush(file);
    printf(".");
    total_len += rcv_len;
    // sys_sleep(1000);
  }
  if (rcv_len < 0) {
    printf("recv error\n");
    goto download_failed;
  }

  rcv_len = recv(sockfd, buf, sizeof(buf), 0);

  printf("recv file size: %d\n", (int)total_len);
  printf("download success\n");

  closesocket(sockfd);
  fclose(file);
  return;
download_failed:
  closesocket(sockfd);
  if (file) {
    fclose(file);
  }
  return;
}
