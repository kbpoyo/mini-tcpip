/**
 * @file ipaddr.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief ip地址模块
 * @version 0.1
 * @date 2024-05-20
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef IPADDR_H
#define IPADDR_H

#include <stdint.h>

#include "net_err.h"

#define IP_ADDR_SIZE 4  // ip地址长度, 暂时只支持ipv4

typedef struct _ipaddr_t {
  enum {
    IPADDR_V4 = 0,  // ipv4地址
  } type;           // ip地址类型

  union {                              // ipv4地址
    uint32_t addr;                     // ip地址
    uint8_t addr_bytes[IP_ADDR_SIZE];  // ip地址字节
  };

} ipaddr_t;

net_err_t ipaddr_from_str(ipaddr_t *dest, const char *src);
ipaddr_t *ipaddr_get_any(void);
int ipaddr_is_match(const ipaddr_t *dest_ipaddr, const ipaddr_t *local_ipaddr,
                    const ipaddr_t *netmask);
/**
 * @brief 判断是否为本地广播地址(255.255.255.255)(全网段进行广播)
 *
 * @param ipaddr
 * @return int
 */
static inline int ipaddr_is_local_broadcast(const ipaddr_t *ipaddr) {
  return ipaddr ? ipaddr->addr == 0xFFFFFFFF : 0;
}

/**
 * @brief 获取ip地址对应的主机号
 *
 * @param ipaddr
 * @param mask
 * @return int
 */
static inline ipaddr_t ipaddr_get_host(const ipaddr_t *ipaddr,
                                       const ipaddr_t *mask) {
  ipaddr_t host;
  host.type = IPADDR_V4;
  host.addr = ipaddr ? (ipaddr->addr & ~mask->addr) : 0;
  return host;
}

/**
 * @brief 获取ip地址所对应的网络号
 *
 * @param ipaddr
 * @param mask
 * @return ipaddr_t
 */
static inline ipaddr_t ipaddr_get_netnum(const ipaddr_t *ipaddr,
                                         const ipaddr_t *mask) {
  ipaddr_t netnum;
  netnum.type = IPADDR_V4;
  netnum.addr = ipaddr ? (ipaddr->addr & mask->addr) : 0;

  return netnum;
}

/**
 * @brief  判断是否为定向广播地址(ip & ~mask = 0xffffffff & ~mask)
 * 定向广播地址及某一子网的广播地址
 * @param ipaddr
 * @return int
 */
static inline int ipaddr_is_direct_broadcast(const ipaddr_t *ipaddr,
                                             const ipaddr_t *mask) {
  ipaddr_t host = ipaddr_get_host(ipaddr, mask);

  return host.addr == (0xffffffff & ~mask->addr);
}

/**
 * @brief 获取ip地址的字节数组
 *
 * @param ipaddr
 * @return const uint8_t*
 */
static inline const uint8_t *ipaddr_get_bytes(const ipaddr_t *ipaddr) {
  return ipaddr ? ipaddr->addr_bytes : (uint8_t *)0;
}

/**
 * @brief 返回ip地址的有效位数(为1的位数)
 *
 * @param ipaddr
 * @return const uint8_t
 */
static inline const uint8_t ipaddr_valid_digits(const ipaddr_t *ipaddr) {
  int cnt = 0;
  uint32_t addr = ipaddr->addr;
  while (addr) {
    if (addr & 0x1) {
      cnt++;
    }
    addr >>= 1;
  }

  return cnt;
}

/**
 * @brief 将字节数组转换为ip地址结构
 *
 * @param dest
 * @param src
 */
static inline void ipaddr_from_bytes(ipaddr_t *dest, const uint8_t *src) {
  dest->type = IPADDR_V4;
  dest->addr = *(uint32_t *)src;
}

/**
 * @brief 将ip地址结构转换为字节数组
 *
 * @param src
 * @param dest
 */
static inline void ipaddr_to_bytes(const ipaddr_t *src, uint8_t *dest) {
  *(uint32_t *)dest = src->addr;
}

/**
 * @brief 复制ip地址
 *
 * @param dest
 * @param src
 */
static inline void ipaddr_copy(ipaddr_t *dest, const ipaddr_t *src) {
  dest->type = src->type;
  dest->addr = src->addr;
}

/**
 * @brief 设置ip地址为任意地址
 *
 * @param ipaddr
 */
static inline void ipaddr_set_any(ipaddr_t *ipaddr) {
  ipaddr->type = IPADDR_V4;
  ipaddr->addr = 0;
}

/**
 * @brief 判断ip地址是否为空地址
 * 
 * @param ipaddr 
 * @return int 
 */
static inline int ipaddr_is_any(const ipaddr_t *ipaddr) {
  return ipaddr->addr == 0;
}

/**
 * @brief 判断ip1和ip2是否相等
 *
 * @param ip1
 * @param ip2
 * @return int
 */
static inline int ipaddr_is_equal(const ipaddr_t *ip1, const ipaddr_t *ip2) {
  return ip1->addr == ip2->addr;
}


#endif  // IPADDR_H