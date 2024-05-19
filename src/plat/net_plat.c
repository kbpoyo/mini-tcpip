
#include "net_plat.h"
#include "dbg.h"
#include "net_cfg.h"

/**
 * @brief 针对特定平台对协议栈进行初始化 
 * 
 * @return net_err_t 
 */
net_err_t net_plat_init(void) {
    dbg_info(DBG_PLAT, "init plat....");
    dbg_info(DBG_PLAT, "init plat ok\n");

    return NET_ERR_OK;
}

