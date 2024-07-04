/**
 * @file ipaddr.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief ip地址模块
 * @version 0.1
 * @date 2024-05-21
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "ipaddr.h"

/**
 * @brief 设置ip地址为任意地址
 *
 * @param ipaddr
 */
void ipaddr_set_any(ipaddr_t *ipaddr) {
  ipaddr->type = IPADDR_V4;
  ipaddr->addr = 0;
}

/**
 * @brief 从字符串解析ip地址
 *
 * @param dest
 * @param src
 * @return net_err_t
 */
net_err_t ipaddr_from_str(ipaddr_t *dest, const char *src) {
  if (dest == (ipaddr_t *)0 || src == (char *)0) {
    return NET_ERR_PARAM;
  }

  // 设置ip地址类型为ipv4
  dest->type = IPADDR_V4;
  dest->addr = 0;

  // 解析ip地址
  uint8_t *p = dest->addr_bytes;
  uint8_t sub_addr = 0;

  char c;
  while ((c = *(src++)) != '\0') {
    if (c >= '0' && c <= '9') {
      sub_addr = sub_addr * 10 + (c - '0');
    } else if (c == '.') {
      *(p++) = sub_addr;  // 写入子地址
      sub_addr = 0;
    } else {
      return NET_ERR_PARAM;
    }
  }

  *p = sub_addr;  // 最后一个子地址

  return NET_ERR_OK;
}

/**
 * @brief 复制ip地址
 *
 * @param dest
 * @param src
 */
void ipaddr_copy(ipaddr_t *dest, const ipaddr_t *src) {
  if (dest == (ipaddr_t *)0 || src == (ipaddr_t *)0) {
    return;
  }

  dest->type = src->type;
  dest->addr = src->addr;
}

/**
 * @brief 获取默认的空ip地址(0.0.0.0)
 *
 * @return ipaddr_t*
 */
ipaddr_t *ipaddr_get_any(void) {
  static const ipaddr_t ipaddr_any = {.type = IPADDR_V4, .addr = 0};

  return (ipaddr_t *)&ipaddr_any;
}

/**
 * @brief 判断ip1和ip2是否相等
 *
 * @param ip1
 * @param ip2
 * @return int
 */
int ipaddr_is_equal(const ipaddr_t *ip1, const ipaddr_t *ip2) {
  return ip1->addr == ip2->addr;
}

/**
 * @brief 将字节数组转换为ip地址结构
 *
 * @param dest
 * @param src
 */
void ipaddr_from_bytes(ipaddr_t *dest, const uint8_t *src) {
  if (dest == (ipaddr_t *)0 || src == (uint8_t *)0) {
    return;
  }

  dest->type = IPADDR_V4;
  dest->addr = *(uint32_t *)src;
}

/**
 * @brief 将ip地址结构转换为字节数组
 *
 * @param src
 * @param dest
 */
void ipaddr_to_bytes(const ipaddr_t *src, uint8_t *dest) {
  if (src == (ipaddr_t *)0 || dest == (uint8_t *)0) {
    return;
  }

  *(uint32_t *)dest = src->addr;
}

int ipaddr_is_match(const ipaddr_t *dest_ipaddr, const ipaddr_t *local_ipaddr,
                    const ipaddr_t *netmask) {
  if (ipaddr_is_equal(dest_ipaddr, local_ipaddr)) {
    // 目的地址与本机地址精准匹配
    return 1;
  }

  if (ipaddr_is_local_broadcast(dest_ipaddr)) {
    // 该目的地址为全网段广播地址，可以和任意本机ip匹配
    return 1;
  }

  if (ipaddr_is_direct_broadcast(dest_ipaddr,
                                 netmask)) {  // 目的地址为一个子网的广播地址
    // 获取ip地址所属网络号
    ipaddr_t dest_netnum = ipaddr_get_netnum(dest_ipaddr, netmask);
    ipaddr_t local_netnum = ipaddr_get_netnum(local_ipaddr, netmask);

    if (ipaddr_is_equal(&dest_netnum, &local_netnum)) {
      // 目的地址与本地ip地址属于同一网段，且为该网段的广播地址，可以与本机任意ip地址匹配
      return 1;
    } 
  }

  return 0;
}