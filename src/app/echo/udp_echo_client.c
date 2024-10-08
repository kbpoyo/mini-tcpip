
// #include <winsock2.h>

#include "udp_echo_client.h"

#include "net_api.h"
#include "sys_plat.h"

int udp_echo_client_start(const char *ip, int port) {
  plat_printf("udp echo client, ip: %s, port: %d\n", ip, port);

  // // 初始化WinSock
  // WSADATA wsaData;
  // WSAStartup(MAKEWORD(2, 2), &wsaData);

  // 创建客户端socket
  int client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (client_socket < 0) {
    plat_printf("udp client: create socket error\n");
    goto client_end;
  }

  // 创建并初始化服务器socket
  struct sockaddr_in server_addr;
  plat_memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  // server_addr.sin_addr.s_addr = inet_addr(ip);
  server_addr.sin_addr.s_addr = inet_addr(ip);
  server_addr.sin_port = htons(port);

  // 客户端与服务器socket连接，只接收指定服务器的数据
  int ret = connect(client_socket, (const struct sockaddr *)&server_addr,
                    sizeof(server_addr));

  char buf[128];
  plat_printf(">>");
  while (fgets(buf, sizeof(buf), stdin)) {
    if (strncmp(buf, "quit", 4) == 0) {
      break;
    }
    // ret = sendto(client_socket, buf, plat_strlen(buf), 0,
    //              (struct sockaddr *)&server_addr, sizeof(server_addr));
    ret = send(client_socket, buf, plat_strlen(buf), 0);
    if (ret < 0) {
      plat_printf("udp client sendto error\n");
      goto client_end;
    }

    plat_memset(buf, 0, sizeof(buf));
    struct sockaddr_in from_addr;
    int from_len = sizeof(from_addr);
    // int len = recvfrom(client_socket, buf, sizeof(buf), 0,
    //                    (struct sockaddr *)&from_addr, &from_len);
    int len = recv(client_socket, buf, sizeof(buf), 0);
    if (len < 0) {
      plat_printf("udp client recv error\n");
      goto client_end;
    }
    buf[len - 1] = '\0';
    plat_printf("recv: %s\n", buf);
    plat_printf(">>");
  }

  close(client_socket);
  return 0;

client_end:
  if (client_socket >= 0) {
    close(client_socket);
  }
  return -1;
}
