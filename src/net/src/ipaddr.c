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
  if (!dest || !src) {
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
      *(p++) = sub_addr;    // 写入子地址
      sub_addr = 0;
    } else {
      return NET_ERR_PARAM;
    }
  }

  *p = sub_addr;    // 最后一个子地址

  return NET_ERR_OK;
}