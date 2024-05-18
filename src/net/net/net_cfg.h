/**
 * @file net_cfg.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 协议栈配置文件
 * @version 0.1
 * @date 2024-05-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef NET_CFG_H
#define NET_CFG_H

// 调试信息相关配置
#define DBG_INIT DBG_LEVEL_INFO
#define DBG_MBLOCK DBG_LEVEL_INFO
#define DBG_FIXQ DBG_LEVEL_INFO
#define DBG_EXMSG DBG_LEVEL_INFO
#define DBG_PKTBUF DBG_LEVEL_INFO

// 消息队列相关配置
#define EXMSG_MSG_CNT 10                    // 消息队列大小
#define EXMSG_LOCKER_TYPE NLOCKER_THREAD    // 使用的锁类型

// 数据包相关配置
#define PKTBUF_LOCKER_TYPE NLOCKER_THREAD // 数据包模块锁类型
#define PKTBUF_BLK_SIZE 128 // 数据包有效载荷大小
#define PKTBUF_BLK_CNT 128   // 数据块池中的数据块数量
#define PKTBUF_BUF_CNT 128   // 数据包池中的数据包数量



#endif  // NET_CFG_H