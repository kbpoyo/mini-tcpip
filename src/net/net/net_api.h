/**
 * @file net_api.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 协议栈提供给应用程序的API
 * @version 0.1
 * @date 2024-07-30
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef NET_API_H
#define NET_API_H

#include "socket.h"
#include "tools.h"

// 字节序转换相关接口
#undef htons // 为了避免本地协议栈重定义，先取消宏定义
#define htons(val) net_htons(val)
#undef ntohs
#define ntohs(val) net_ntohs(val)
#undef htonl
#define htonl(val) net_htonl(val)
#undef ntohl
#define ntohl(val) net_ntohl(val)

// ip地址转换相关接口
char *net_inet_ntoa(struct net_in_addr addr);
uint32_t net_inet_addr(const char *ip_str);
int net_inet_pton(int family, const char *strptr, void *addrptr);
const char *net_inet_ntop(int family, const void *addrptr, char *strptr, size_t len);

#undef inet_ntoa
#define inet_ntoa(addr) net_inet_ntoa(addr)
#undef inet_addr
#define inet_addr(ip_str) net_inet_addr(ip_str)
#undef inet_pton
#define inet_pton(family, strptr, addrptr) net_inet_pton(family, strptr, addrptr)
#undef inet_ntop
#define inet_ntop(family, addrptr, strptr, len) net_inet_ntop(family, addrptr, strptr, len)


// socket相关接口
#undef sockaddr_in
#define sockaddr_in net_sockaddr_in
#undef sockaddr
#define sockaddr net_sockaddr
#undef timeval
#define timeval net_timeval
#undef socket
#define socket(family, type, protocol) net_socket(family, type, protocol)
#undef sendto
#define sendto(sock, buf, buf_len, flags, dest, dest_len) \
  net_sendto(sock, buf, buf_len, flags, dest, dest_len)
#undef recvfrom
#define recvfrom(sock, buf, buf_len, flags, src, src_len) \
  net_recvfrom(sock, buf, buf_len, flags, src, src_len)
#undef close
#define close(sock) net_close(sock)
#undef setsockopt
#define setsockopt(sock, level, optname, optval, optlen) \
  net_setsockopt(sock, level, optname, optval, optlen)




#endif  // NET_API_H