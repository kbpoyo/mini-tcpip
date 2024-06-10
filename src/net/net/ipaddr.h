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

#define IPV4_ADDR_SIZE 4  // ip地址长度

typedef struct _ipaddr_t {
  enum {
    IPADDR_V4 = 0,  // ipv4地址
  } type;           // ip地址类型

  union { // ipv4地址
    uint32_t addr;                    // ip地址
    uint8_t addr_bytes[IPV4_ADDR_SIZE];  // ip地址字节
  };

} ipaddr_t;

void ipaddr_set_any(ipaddr_t *ipaddr);

net_err_t ipaddr_from_str(ipaddr_t *dest, const char *src);
void ipaddr_copy(ipaddr_t *dest, const ipaddr_t *src);
ipaddr_t *ipaddr_get_any(void);
int ipaddr_is_equal(const ipaddr_t *ip1, const ipaddr_t *ip2);

#endif  // IPADDR_H