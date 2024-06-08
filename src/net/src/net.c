#include "net.h"

#include "dbg.h"
#include "ether.h"
#include "exmsg.h"
#include "loop.h"
#include "net_err.h"
#include "net_plat.h"
#include "net_sys.h"
#include "netif.h"
#include "pktbuf.h"
#include "timer.h"
#include "tools.h"

net_err_t net_init(void) {
  net_plat_init();

  // 初始化tools模块
  tools_module_init();

  // 初始化数据包模块
  pktbuf_module_init();

  // 初始化消息队列工作模块
  exmsg_module_init();

  // 初始化定时器模块
  net_timer_module_init();

  // 初始化网络接口模块
  netif_module_init();

  // 初始化环回接口模块
  loop_module_init();

  // 初始化以太网协议层
  ether_module_init();

  return NET_ERR_OK;
}

net_err_t net_start(void) {
  exmsg_start();
  dbg_info(DBG_INIT, "net is running");

  return NET_ERR_OK;
}
