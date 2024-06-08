/**
 * @file tools.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 工具函数
 * @version 0.1
 * @date 2024-05-26
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "tools.h"
#include "dbg.h"
#include "net_cfg.h"


/**
 * @brief 检测系统是否为小端字节序
 * 
 * @return int 
 */
static int is_litte_endian(void) {
//   union {
//     uint32_t i;
//     uint8_t c[4];
//   } u = {0x12345678};
//   return (u.c[0] == 0x78);

  return *(uint16_t*)"abc" == 0x6261;   //大端读出来是0x6162

}

/**
 * @brief 初始化工具函数模块，检查系统字节序
 * 
 * @return net_err_t 
 */
net_err_t tools_module_init(void) {
  dbg_info(DBG_TOOLS, "init tools module....");

  if (is_litte_endian() != SYS_ENDIAN_LITTLE) {
    dbg_error(DBG_TOOLS, "check system endian error.");
  }

  dbg_info(DBG_TOOLS, "init tools module ok.");
  return NET_ERR_OK;
}