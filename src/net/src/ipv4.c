/**
 * @file ipv4.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief ipv4协议模块
 * @version 0.1
 * @date 2024-06-15
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "ipv4.h"
#include "dbg.h"

/**
 * @brief 初始化ipv4协议模块
 * 
 * @return net_err_t 
 */
net_err_t ipv4_module_init(void) {
    dbg_info(DBG_IPV4, "init ipv4 module....");

    


    dbg_info(DBG_IPV4, "init ipv4 module ok.");
    return NET_ERR_OK;
}