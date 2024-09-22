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

// 系统相关配置
#define SYS_ENDIAN_LITTLE 1  // 小端模式

// 协议栈相关配置
#define NET_MAC_FRAME_MTU 1500  // MAC帧最大传输单元
#define NET_MAC_FRAME_MAX_SIZE \
  (6 + 6 + 2 +                 \
   NET_MAC_FRAME_MTU)  // MAC帧最大大小, 6B目的地址 + 6B源地址 + 2B类型 +
                       // 1500B数据, (4B的校验码由网卡自动添加)

// 调试信息相关配置
#define DBG_PLAT DBG_LEVEL_ERROR
#define DBG_INIT DBG_LEVEL_INFO
#define DBG_MBLOCK DBG_LEVEL_ERROR
#define DBG_FIXQ DBG_LEVEL_ERROR
#define DBG_EXMSG DBG_LEVEL_INFO
#define DBG_PKTBUF DBG_LEVEL_WARN
#define DBG_NETIF DBG_LEVEL_WARN
#define DBG_LOOP DBG_LEVEL_WARN
#define DBG_PCAP DBG_LEVEL_ERROR
#define DBG_ETHER DBG_LEVEL_ERROR
#define DBG_TOOLS DBG_LEVEL_WARN
#define DBG_TIMER DBG_LEVEL_ERROR
#define DBG_ARP DBG_LEVEL_WARN
#define DBG_IPV4 DBG_LEVEL_ERROR
#define DBG_ICMPV4 DBG_LEVEL_ERROR
#define DBG_ROUTE DBG_LEVEL_INFO
#define DBG_SOCKET DBG_LEVEL_INFO
#define DBG_SOCKRAW DBG_LEVEL_INFO

// 消息队列相关配置
#define EXMSG_MSG_CNT 10                  // 消息队列大小
#define EXMSG_LOCKER_TYPE NLOCKER_THREAD  // 使用的锁类型

// 数据包相关配置
#define PKTBUF_LOCKER_TYPE NLOCKER_THREAD  // 数据包模块锁类型
#define PKTBUF_BLK_SIZE 128                // 数据包有效载荷大小
#define PKTBUF_BLK_CNT 128                 // 数据块池中的数据块数量
#define PKTBUF_BUF_CNT 128                 // 数据包池中的数据包数量

// 网络接口相关配置
#define NETIF_HWADDR_SIZE 10   // 网络接口硬件地址长度
#define NETIF_NAME_SIZE 10     // 网络接口名称长度
#define NETIF_RECV_BUFSIZE 50  // 网络接口接收缓冲区大小
#define NETIF_SEND_BUFSIZE 50  // 网络接口发送缓冲区大小
#define NETIF_MAX_CNT 10       // 网络接口最大数量

// ARP模块相关配置
#define ARP_CACHE_TBL_CNT 50  // arp缓存表大小
#define ARP_WAIT_PKT_MAXCNT 5  // arp缓存表对应的等待数据包的最大数量
#define ARP_CACHE_SCAN_PERIOD 1  // arp缓存表的扫描定时器的超时时间(s)
#define ARP_ENTRY_WAITING_TMO \
  (3 * ARP_CACHE_SCAN_PERIOD)  // 待解析的arp缓存表项的超时时间(s)
#define ARP_ENTRY_RESOLVED_TMO \
  (5 * ARP_CACHE_SCAN_PERIOD)  // 已解析的arp缓存表项的超时时间(s)
#define ARP_ENTRY_RETRY_CNT 5  // arp缓存表项允许的重复请求次数

// IPv4模块相关配置
#define IPV4_DEFAULT_TTL 64      // ipv4数据包默认生存跳数
#define IPV4_FRAG_MAXCNT 10      // ipv4分片缓存数组大小
#define IPV4_FRAG_BUF_MAXCNT 10  // 每个分片允许缓存的最大数据包数量
#define IPV4_FRAG_SCAN_PERIOD 1  // ipv4分片缓存表的扫描周期(s)
#define IPV4_FRAG_TMO \
  (10 * IPV4_FRAG_SCAN_PERIOD)  // ipv4分片缓存表项的超时时间(s)

// ROUTE(路由)模块相关配置
#define ROUTE_ENTRY_MAXCNT 20  // 路由表大小

// 原始socket模块(sock_raw)相关配置
#define SOCKRAW_MAXCNT 10        // 原始socket对象表大小
#define SOCKRAW_RECV_MAXCNT 128  // 接收缓冲区链表最大长度

#endif  // NET_CFG_H