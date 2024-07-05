/**
 * @file icmpv4.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief icmpv4协议处理模块
 * @version 0.1
 * @date 2024-07-05
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "icmpv4.h"
#include "dbg.h"
#include "net_err.h"


/**
 * @brief 初始化icmpv4协议处理模块
 * 
 * @return net_err_t 
 */
net_err_t icmpv4_module_init(void) {
    dbg_info(DBG_ICMPV4, "init icmpv4 module....");

    dbg_info(DBG_ICMPV4, "init icmpv4 module ok.");
    return NET_ERR_OK;
}

/**
 * @brief 接收一个icmpv4数据包
 * 
 * @param dest_ipaddr 数据包目的ip地址
 * @param src_ipaddr 数据包源ip地址
 * @param buf 数据包
 * @return net_err_t 
 */
net_err_t icmpv4_recv(const ipaddr_t *dest_ipaddr, const ipaddr_t *src_ipaddr, pktbuf_t *buf) {
    return NET_ERR_OK;
}