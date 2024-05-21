#include "net.h"
#include "net_err.h"
#include "exmsg.h"
#include "net_plat.h"
#include "net_sys.h"
#include "pktbuf.h"
#include "dbg.h"
#include "netif.h"
#include "loop.h"


net_err_t net_init(void) {

    net_plat_init();

    // 初始化数据包模块
    pktbuf_module_init();

    // 初始化网络接口模块
    netif_module_init();

    // 初始化环回接口模块
    loop_module_init();

    // 初始化消息队列工作模块
    exmsg_module_init();

    return NET_ERR_OK;
}

net_err_t net_start(void) {

    exmsg_start();
    dbg_info(DBG_INIT, "net is running\n");

    return NET_ERR_OK;
}

