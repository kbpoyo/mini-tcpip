/**
 * @file tools.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 工具函数
 * @version 0.1
 * @date 2024-05-26
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef TOOLS_H
#define TOOLS_H

#include <stdint.h>
#include "net_cfg.h"
#include "net_err.h"
#include "pktbuf.h"
#include "ipaddr.h"

/**
 * @brief u16类型数据字节序转换
 *
 * @param val
 * @return uint16_t
 */
static inline uint16_t swap_u16(uint16_t val) {
  return (((val >> 8) & 0xff) | ((val << 8) & 0xff00));
}

/**
 * @brief u32类型数据字节序转换
 *
 * @param val
 * @return uint32_t
 */

static inline uint32_t swap_u32(uint32_t val) {
  return (((val >> 24) & 0xff) | ((val >> 8) & 0xff00) |
          ((val << 8) & 0xff0000) | ((val << 24) & 0xff000000));
}

#if SYS_ENDIAN_LITTLE

#define net_htons(val) swap_u16(val)
#define net_ntohs(val) swap_u16(val)
#define net_htonl(val) swap_u32(val)
#define net_ntohl(val) swap_u32(val)

#else

#define net_htons(val) (val)
#define net_htonl(val) (val)
#define net_ntohs(val) (val)
#define net_ntohl(val) (val)

#endif


net_err_t tools_module_init(void);


uint16_t tools_checksum16(const void *data, uint16_t len, uint32_t pre_sum, int offset, int is_take_back);

uint16_t tools_checksum16_pseudo_head(pktbuf_t *buf, const ipaddr_t *dest_ip,
                                       const ipaddr_t *src_ip, uint8_t proto);

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))


#endif  // TOOLS_H