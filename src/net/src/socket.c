/**
 * @file socket.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 提供给应用程序的socket相关编程接口
 * @version 0.1
 * @date 2024-07-31
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "socket.h"
#include "exmsg.h"
#include "sock.h"

/**
 * @brief 打开一个socket对象
 * 
 * @param family 
 * @param type 
 * @param protocol 
 * @return int 
 */
int net_socket(int family, int type, int protocol) {

    // 封装socket创建请求参数
    sock_req_t sock_req;
    sock_req.create.family = family;
    sock_req.create.type = type;
    sock_req.create.protocol = protocol;
    sock_req.sock_fd = -1;


    // 调用消息队列工作线程执行socket创建请求
    net_err_t err = exmsg_func_exec(sock_req_creat, &sock_req);
    if (err != NET_ERR_OK) {
        dbg_error(DBG_SOCKET, "socket create failed.\n");
        return -1;
    }

    // 返回socket文件描述符
    return sock_req.sock_fd;
}