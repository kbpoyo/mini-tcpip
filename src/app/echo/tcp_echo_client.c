#include "tcp_echo_client.h"

// #include <WinSock2.h>
#include "net_api.h"
#include "sys_plat.h"

int tcp_echo_client_start(const char *ip, int port) {
  plat_printf("tcp echo client, ip: %s, port: %d\n", ip, port);

  // 初始化WinSock
  // WSADATA wsaData;
  // WSAStartup(MAKEWORD(2, 2), &wsaData);

  // 创建客户端socket
  int client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket < 0) {
    plat_printf("create socket error\n");
    goto client_end;
  }

  // 创建并初始化服务器socket
  struct sockaddr_in server_addr;
  plat_memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ip);
  server_addr.sin_port = htons(port);

  // 连接服务器
  int ret = connect(client_socket, (const struct sockaddr *)&server_addr,
                    sizeof(server_addr));
  if (ret < 0) {
    plat_printf("connect error\n");
    goto client_end;
  }
  
#if 0 //测试 tcp连接管理部分
  getchar();
  close(client_socket);
  return 0;
#endif

#if 1 //测试tcp数据传输部分
  char sbuf[4096];
  for (int i = 0; i < sizeof(sbuf); i++) {
    sbuf[i] = 'a' + i % 26;
  }

  for (int i = 0; i < 10; i++) {
    ssize_t size = send(client_socket, sbuf, sizeof(sbuf), 0);
    if (size < 0) {
      plat_printf("send error\n");
      break;
    }
  }

  getchar();
  close(client_socket);
  return 0;
#endif

  char buf[128];
  plat_printf(">>");
  while (fgets(buf, sizeof(buf), stdin)) {
    ret = send(client_socket, buf, plat_strlen(buf), 0);
    if (ret < 0) {
      plat_printf("send error\n");
      goto client_end;
    }

    plat_memset(buf, 0, sizeof(buf));
    int len = recv(client_socket, buf, sizeof(buf), 0);
    if (len < 0) {
      plat_printf("recv error\n");
      goto client_end;
    }
    plat_printf("recv: %s", buf);
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