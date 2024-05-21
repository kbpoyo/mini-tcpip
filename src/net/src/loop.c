/**
 * @file loop.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 环回接口模块
 * @version 0.1
 * @date 2024-05-21
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "loop.h"
#include "dbg.h"
#include "netif.h"

/**
 * @brief 打开环回网卡, 将网络接口进一步初始化为环回接口
 * 
 * @param netif 
 * @return net_err_t 
 */
static net_err_t loop_open(netif_t *netif, void *data) {
    // 设置网络接口类型为环回接口
    netif->type = NETIF_TYPE_LOOP;

    return NET_ERR_OK;
}

/**
 * @brief 关闭环回网卡
 * 
 * @param netif 
 * @return net_err_t 
 */
static void loop_close(netif_t *netif) {
}

/**
 * @brief 环回网卡发送数据
 * 
 * @param netif 
 * @param data 
 * @param len 
 * @return net_err_t 
 */
static net_err_t loop_send(netif_t *netif) {
    return NET_ERR_OK;
}

/**
 * @brief 环回接口的操作方法
 * 
 */
static const netif_ops_t loop_ops = {
    .open = loop_open,
    .close = loop_close,
    .send = loop_send,
};

/**
 * @brief 初始化环回接口模块
 * 
 * @return net_err_t 
 */
net_err_t loop_module_init(void) {

    dbg_info(DBG_LOOP, "init loop module....");

    // 打开一个环回网络接口
    netif_t *netif = netif_open("loop", &loop_ops, (void *)0);
    if (netif == (netif_t *)0) {
        dbg_error(DBG_LOOP, "no memory for loop netif.\n");
        return NET_ERR_MEM;
    }

    // 设置环回接口的ip地址和子网掩码
    ipaddr_t ip, mask;
    ipaddr_from_str(&ip, "127.0.0.1");
    ipaddr_from_str(&mask, "255.0.0.0");





    dbg_info(DBG_LOOP, "init loop module ok.");

    return NET_ERR_OK;
}