
#include <stdio.h>
#include <WinSock2.h>

#include "sys_plat.h"
#include "tcp_echo_server.h"


int tcp_echo_server_start(int port) {
    
    plat_printf("tcp echo server, port: %d\n", port);

    // 初始化WinSock
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // 创建服务器socket
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        plat_printf("create socket error\n");
        goto server_end;
    }

    // 创建并初始化服务器socket地址
    struct sockaddr_in server_addr;
    plat_memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    // 绑定服务器socket地址
    int ret = bind(server_socket, (const struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret < 0) {
        plat_printf("bind error\n");
        goto server_end;
    }

    // 设置监听队列长度
    listen(server_socket, 5);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        SOCKET client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
        if (client_socket < 0) {
            plat_printf("accept error\n");
            break;
        }

        plat_printf("tcp echo server: connect ip: %s, port: %d\n", 
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    

        char buf[128];
        size_t size;
        while ((size = recv(client_socket, buf, sizeof(buf), 0)) > 0) {
            buf[size] = 0;
            plat_printf("recv: %s", buf);
            ret = send(client_socket, buf, size, 0);
            if (ret < 0) {
                plat_printf("send error\n");
                break;
            }
        }

        closesocket(client_socket);
    }

    closesocket(server_socket);
    return 0;
server_end:
    if (server_socket >= 0) {
        closesocket(server_socket);
    }
    return -1;

}