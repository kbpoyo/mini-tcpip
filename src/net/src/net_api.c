/**
 * @file net_api.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 协议栈提供给应用程序的API
 * @version 0.1
 * @date 2024-07-30
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "net_api.h"

#define IPV4_ADDR_STR_SIZE 16

/**
 * @brief 将ip地址结构转换为字符串(点分十进制)
 * 该函数是线程不安全的(不可重入)
 *
 * @param addr
 * @return char*
 */
char *net_inet_ntoa(struct net_in_addr ip_addr) {
  static char ip_buf[IPV4_ADDR_STR_SIZE];

  plat_sprintf(ip_buf, "%d.%d.%d.%d", ip_addr.s_addr0, ip_addr.s_addr1,
               ip_addr.s_addr2, ip_addr.s_addr3);

  return ip_buf;
}

/**
 * @brief 将点分十进制ip地址转换为32位整数
 *
 * @param ip_str
 * @return uint32_t
 */
uint32_t net_inet_addr(const char *ip_str) {
  if (!ip_str) {  // 字符串为空，返回默认值
    return INADDR_ANY;
  }

  ipaddr_t ipaddr;
  ipaddr_from_str(&ipaddr, ip_str);

  return ipaddr.addr;
}

/**
 * @brief
 * 根据ip协议版本将点分十进制ip地址字符串(strptr)转换到ip地址结构(addrptr)中
 *
 * @param family 协议族版本
 * @param strptr IP地址字符串指针
 * @param addrptr IP地址结构指针
 * @return int
 */
int net_inet_pton(int family, const char *strptr, void *addrptr) {
  if ((family != AF_INET) || !strptr || !addrptr) {
    return -1;
  }

  struct net_in_addr *in_addr = (struct net_in_addr *)addrptr;

  ipaddr_t ipaddr;
  ipaddr_from_str(&ipaddr, strptr);
  in_addr->s_addr = ipaddr.addr;

  return 0;
}
/**
 * @brief
 * 根据ip协议版本将ip地址结构(addrptr)转换为点分十进制ip地址字符串(strptr)
 *
 * @param family ip协议族版本
 * @param addrptr ip地址结构指针
 * @param strptr ip地址字符串指针
 * @param len 字符串长度
 * @return const char*
 */
const char *net_inet_ntop(int family, const void *addrptr, char *strptr,
                          size_t len) {
  if ((family != AF_INET) || !addrptr || !strptr || !len) {
    return NULL;
  }

  struct net_in_addr *in_addr = (struct net_in_addr *)addrptr;

  char ipbuf[IPV4_ADDR_STR_SIZE];
  plat_sprintf(ipbuf, "%d.%d.%d.%d", in_addr->s_addr0, in_addr->s_addr1,
               in_addr->s_addr2, in_addr->s_addr3);
  plat_memcpy(strptr, ipbuf, len - 1);
  strptr[len - 1] = '\0';

  return strptr;
}