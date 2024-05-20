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

#define IPADDR_SIZE 4  // ip地址长度

typedef struct _ipaddr_t {
  enum {
    IPADDR_V4 = 0,  // ipv4地址
  } type;           // ip地址类型

  union {
    uint32_t addr;                    // ip地址
    uint8_t addr_bytes[IPADDR_SIZE];  // ip地址字节
  };

} ipaddr_t;

#endif  // IPADDR_H