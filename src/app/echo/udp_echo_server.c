

#include "udp_echo_server.h"

#include "net_api.h"
#include "sys_plat.h"

void udp_echo_server_run(void *arg) {
  int port = (int)arg;

  // 初始化WinSock
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);

  // 创建客户端socket
  int server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (server_socket < 0) {
    plat_printf("udp client: create socket error\n");
    goto client_end;
  }

  // 创建并初始化服务器socket
  struct sockaddr_in server_addr;
  plat_memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  // 绑定服务器socket地址
  //   int ret = bind(server_socket, (const struct sockaddr *)&server_addr,
  //                  sizeof(server_addr));
  int ret = 1;
  if (ret < 0) {
    plat_printf("udp echo server bind error\n");
    goto client_end;
  }

  char buf[128];
  plat_memset(buf, 0, sizeof(buf));
  struct sockaddr_in from_addr;
  int from_len = sizeof(from_addr);
  int len = 0;
  while ((len = recvfrom(server_socket, buf, sizeof(buf), 0,
                         (struct sockaddr *)&from_addr, &from_len)) > 0) {
    ret = sendto(server_socket, buf, len, 0, (struct sockaddr *)&from_addr,
                 from_len);
    if (ret < 0) {
      plat_printf("udp server sendto error\n");
      goto client_end;
    }

    plat_memset(buf, 0, sizeof(buf));
  }

  closesocket(server_socket);
  sys_thread_exit(0);

client_end:
  if (server_socket >= 0) {
    closesocket(server_socket);
  }
  sys_thread_exit(0);
}

void udp_echo_server_start(int port) {
  plat_printf("udp echo server, port: %d\n", port);
  sys_thread_create(udp_echo_server_run, (void *)port);
}