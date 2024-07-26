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

  return *(uint16_t *)"abc" == 0x6261;  // 大端读出来是0x6162
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

/**
 * @brief 计算起始地址为data，长度为len个字节的数据的16位校验和
 *  校验和时机:
 *      接收到数据包时，保持头部字段的原有字节序，计算校验和，如果校验和为0，数据包没有损坏
 *      发送数据包时，将头部字段转换为网络字节序，计算校验和，并保持校验和字段原有字节序将其填入数据包后发送
 * 该计算方法计算出的校验和若在小端设备上为0xAABB，则在大端设备上为0xBBAA，所以当小端结果为0时，大端结果也为0，不需要转换字节序再计算校验和
 * @param data 数据起始地址
 * @param len 数据长度
 * @param pre_sum 起始校验和
 * @param offset 当前已计算字节量
 * @param is_take_back 是否对校验和取反
 * @return uint16_t
 */
uint16_t tools_checksum16(const void *data, uint16_t len,
                          uint32_t pre_sum, int offset, int is_take_back) {
  const uint16_t *data_ptr = (const uint16_t *)data;
  uint32_t checksum = pre_sum;
  // 当前已计算字节量为奇数，即当前起始地址(data)的第一个字节
  // 应与上一段数据(计算pre_sum)的最后一个字节组成一个16位数据
  if (offset & 0x1) { 
    // 上一段数据的最后一个字节已加在pre_sum的低8位
    // 当前数据的第一个字节应加在pre_sum的高8位
    checksum += *(uint8_t *)data_ptr << 8;
    data_ptr = (const uint16_t *)((const uint8_t *)data_ptr + 1);
    len--;
  }

  while (len > 1) {  // 每次读取两个字节即16位
    checksum += *(data_ptr++);
    len -= 2;
  }

  if (len) {  // 如果数据长度为奇数，最后一个字节单独处理
    checksum += *(uint8_t *)data_ptr;
  }

  uint16_t high = 0;
  // 校验和最终大小为16位， 如果校验和有进位，将进位加到低16位
  // 直到高16位为0
  while ((high = checksum >> 16) != 0) {
    checksum = high + (checksum & 0xffff);
  }

  // 判断是否取反
  return is_take_back ? (uint16_t)~checksum : (uint16_t)checksum;
}