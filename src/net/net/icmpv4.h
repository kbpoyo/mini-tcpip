/**
 * @file icmpv4.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief icmpv4协议处理模块
 * @version 0.1
 * @date 2024-07-05
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef ICMPV4_H
#define ICMPV4_H

#include "ipaddr.h"
#include "pktbuf.h"

net_err_t icmpv4_module_init(void);
net_err_t icmpv4_recv(const ipaddr_t *dest_ipaddr, const ipaddr_t *src_ipaddr, pktbuf_t *buf);

#endif  // ICMPV4_H