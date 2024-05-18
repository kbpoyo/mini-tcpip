#include "net.h"
#include "net_err.h"
#include "exmsg.h"
#include "net_plat.h"
#include "net_sys.h"
#include "pktbuf.h"
#include "dbg.h"


net_err_t net_init(void) {

    net_plat_init();

    // 初始化数据包模块
    pktbuf_init();

    exmsg_init();

    return NET_ERR_OK;
}

net_err_t net_start(void) {

    exmsg_start();
    dbg_info(DBG_INIT, "net is running\n");

    return NET_ERR_OK;
}

