/**
 * @file ipv4.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief ipv4协议模块
 * @version 0.1
 * @date 2024-06-15
 *  ipv4分片缓存表采取LRU算法, 每次访问一个表项时，都将其移动到链表头，
 * 在空间不够需要释放一个表项时，从链表尾部释放，
 * 这样不仅可以防止最近被访问的表项被释放，还可以在下一次对其进行快速查找
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "ipv4.h"

#include "dbg.h"
#include "ether.h"
#include "icmpv4.h"
#include "mblock.h"
#include "protocol.h"
#include "tools.h"

static ipv4_frag_t
    ipv4_frag_arr[IPV4_FRAG_ARR_SIZE];  // ipv4分片数组(分片内存池)
static mblock_t ipv4_frag_mblock;       // ipv4分片内存池管理对象
static nlist_t ipv4_frag_list;  // ipv4分片链表，记录已分配的分片

#if DBG_DISP_ENABLED(DBG_IPV4)

/**
 * @brief 打印ipv4数据包
 *
 * @param ipv4_pkt
 */
static void ipv4_pkt_display(const ipv4_pkt_t *ipv4_pkt) {
  plat_printf("---------------- ipv4 packet ----------------\n");

  // 打印ipv4头部信息
  const ipv4_hdr_t *hdr = &ipv4_pkt->hdr;
  plat_printf("\tversion: %d\n", hdr->version);
  plat_printf("\thead len: %d\n", ipv4_get_hdr_size(ipv4_pkt));
  plat_printf("\ttotal len: %d\n", hdr->total_len);
  plat_printf("\tid: %d\n", hdr->id);
  plat_printf("\tfrag more: %d\n", hdr->frag_more);
  plat_printf("\tfrag disable: %d\n", hdr->frag_disable);
  plat_printf("\tfrag offset: %d\n", hdr->frag_offset);
  plat_printf("\tttl: %d\n", hdr->ttl);
  char *tran_proto = 0;
  switch (hdr->tran_proto) {
    case NET_PROTOCOL_ICMPv4:
      tran_proto = "icmpv4";
      break;
    case NET_PROTOCOL_UDP:
      tran_proto = "udp";
      break;
    case NET_PROTOCOL_TCP:
      tran_proto = "tcp";
      break;
    default:
      tran_proto = "unknown";
      break;
  }
  plat_printf("\ttranspart protocol: (%d) %s\n", hdr->tran_proto, tran_proto);
  plat_printf("\theadr checksum: 0x%04x\n", hdr->hdr_chksum);
  ipaddr_t ipaddr;
  ipaddr_from_bytes(&ipaddr, hdr->src_ip);
  netif_dum_ip("\tsrc ip: ", &ipaddr);
  ipaddr_from_bytes(&ipaddr, hdr->dest_ip);
  netif_dum_ip("\tdest ip: ", &ipaddr);

  plat_printf("\n---------------------------------------------\n");
}

#else

#define ipv4_pkt_display(ipv4_pkt)
#endif

static inline int ipv4_get_id(void) {
  static int id = 0;
  return id++;
}

/**
 * @brief 初始化ipv4分片内存管理对象
 *
 * @return net_err_t
 */
static net_err_t ipv4_frag_init(void) {
  // 初始化ipv4分片内存池管理对象
  // 该内存管理对象只用于ipv4模块，且ipv4模块只由工作线程一个线程访问，无需加锁
  net_err_t err =
      mblock_init(&ipv4_frag_mblock, ipv4_frag_arr, sizeof(ipv4_frag_t),
                  IPV4_FRAG_ARR_SIZE, NLOCKER_NONE);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_IPV4, "init ipv4 frag mblock failed.");
    return err;
  }

  // 初始化ipv4分片链表
  nlist_init(&ipv4_frag_list);

  return NET_ERR_OK;
}

/**
 * @brief 释放ipv4分片对象缓存的数据包
 *
 * @param frag
 */
static void ipv4_frag_buf_free(ipv4_frag_t *frag) {
  // 释放分片对象的数据包链表
  nlist_node_t *node = 0;
  while ((node = nlist_remove_first(&frag->buf_list))) {
    pktbuf_t *buf = nlist_entry(node, pktbuf_t, node);  //!!! 获取数据包
    pktbuf_free(buf);                                   //!!! 释放数据包
  }
}

/**
 * @brief 分配一个ipv4分片对象
 *
 * @return ipv4_frag_t*
 */
static ipv4_frag_t *ipv4_frag_alloc(void) {
  // 从ipv4分片内存池中分配一个内存块, 无需等待
  ipv4_frag_t *frag = (ipv4_frag_t *)mblock_alloc(&ipv4_frag_mblock, -1);
  if (frag == (ipv4_frag_t *)0) {
    // 无法分配内存块，删除最早的分片对象，并重新使用
    frag = nlist_entry(nlist_remove_last(&ipv4_frag_list), ipv4_frag_t, node);
    // 释放分片对象的数据包链表
    ipv4_frag_buf_free(frag);
  }

  return frag;
}

/**
 * @brief 释放一个ipv4分片对象
 *
 * @param frag
 */
static void ipv4_frag_free(ipv4_frag_t *frag) {
  // 释放分片对象的数据包链表
  ipv4_frag_buf_free(frag);

  // 从分片链表中移除分片对象
  nlist_remove(&ipv4_frag_list, &frag->node);

  // 释放分片对象
  mblock_free(&ipv4_frag_mblock, frag);
}

/**
 * @brief 初始化ipv4协议模块
 *
 * @return net_err_t
 */
net_err_t ipv4_module_init(void) {
  dbg_info(DBG_IPV4, "init ipv4 module....");

  // 初始化ipv4分片内存管理对象
  net_err_t err = ipv4_frag_init();
  if (err != NET_ERR_OK) {
    dbg_error(DBG_IPV4, "init ipv4 frag failed.");
    return err;
  }

  dbg_info(DBG_IPV4, "init ipv4 module ok.");
  return NET_ERR_OK;
}

/**
 * @brief 检测ipv4数据包的正确性
 *
 * @param ipv4_pkt
 * @param pkt_size
 * @param netif
 * @return net_err_t
 */
static net_err_t ipv4_pkt_check(ipv4_pkt_t *ipv4_pkt, uint16_t pkt_size) {
  // 检查ipv4版本号
  if (ipv4_pkt->hdr.version != IPV4_VERSION) {
    dbg_warning(DBG_IPV4, "ipv4 version error.");
    return NET_ERR_IPV4;
  }

  // 检查ipv4头部长度
  int hdr_size = ipv4_get_hdr_size(ipv4_pkt);
  if (hdr_size < sizeof(ipv4_hdr_t)) {
    dbg_warning(DBG_IPV4, "ipv4 header size error.");
    return NET_ERR_IPV4;
  }

  // 检查ipv4数据包总长度
  int total_size = net_ntohs(ipv4_pkt->hdr.total_len);
  if (total_size < hdr_size || total_size > pkt_size) {
    dbg_warning(DBG_IPV4, "ipv4 total size error.");
    return NET_ERR_IPV4;
  }

  // 检查头部校验和
  if (ipv4_pkt->hdr.hdr_chksum) {
    //* 进行校验和计算时，ip头部要保持原有字节序
    uint16_t chksum = tools_checksum16(&ipv4_pkt->hdr, hdr_size, 0, 1);
    if (chksum != 0) {
      dbg_warning(DBG_IPV4, "ipv4 header checksum error.");
      return NET_ERR_IPV4;
    }
  }

  return NET_ERR_OK;
}

/**
 * @brief 将ipv4数据包头部从网络字节序转换为主机字节序
 *
 * @param hdr
 */
static void ipv4_hdr_ntoh(ipv4_hdr_t *hdr) {
  hdr->total_len = net_ntohs(hdr->total_len);
  hdr->id = net_ntohs(hdr->id);
  hdr->flags_frag_offset = net_ntohs(hdr->flags_frag_offset);
}

/**
 * @brief 将ipv4数据包头部从主机字节序转换为网络字节序
 *
 * @param hdr
 */
static void ipv4_hdr_hton(ipv4_hdr_t *hdr) {
  hdr->total_len = net_htons(hdr->total_len);
  hdr->id = net_htons(hdr->id);
  hdr->flags_frag_offset = net_htons(hdr->flags_frag_offset);
}

/**
 * @brief 处理接收到的普通ipv4数据包，即不需要进行分片处理
 * 主要完成多路分解处理，根据数据包协议字段将数据包提供给上层协议处理
 * 如：传输层(TCP, UDP) 和 同属于网络层的ICMP协议
 *
 * @param netif 接受到数据包的网络接口
 * @param buf ipv4数据包
 */
static net_err_t ipv4_handle_normal(const netif_t *netif, pktbuf_t *buf) {
  net_err_t err = NET_ERR_OK;

  // 获取ipv4数据包结构
  ipv4_pkt_t *pkt = (ipv4_pkt_t *)pktbuf_data_ptr(buf);

  // 根据数据包的协议字段将其提供给上一层协议处理
  switch (pkt->hdr.tran_proto) {
    case NET_PROTOCOL_ICMPv4: {
      ipaddr_t src_ipaddr;
      ipaddr_from_bytes(&src_ipaddr, pkt->hdr.src_ip);
      // 当前网络接口已匹配该包的目的地址，直接使用接口地址即可
      err = icmpv4_recv(&netif->ipaddr, &src_ipaddr, buf);  //!!! 数据包传递
      if (err != NET_ERR_OK) {
        dbg_warning(DBG_IPV4, "icmpv4 recv failed.");
        return err;
      }
    } break;

    case NET_PROTOCOL_UDP: {  // TODO: 测试ICMP不可达报文
      dbg_info(DBG_IPV4, "recv UDP packet.");
      ipaddr_t src_ipaddr;
      ipv4_hdr_hton(&pkt->hdr);  // 将头部转换为网络字节序
      ipaddr_from_bytes(&src_ipaddr, pkt->hdr.src_ip);
      icmpv4_make_unreach(&src_ipaddr, &netif->ipaddr, ICMPv4_CODE_UNREACH_PORT,
                          buf);
      return NET_ERR_IPV4;
    } break;

    case NET_PROTOCOL_TCP: {
      dbg_info(DBG_IPV4, "recv TCP packet.");
    } break;

    default: {
      dbg_warning(DBG_IPV4, "unknown transport layer protocol!");
    } break;
  }

  return NET_ERR_OK;
}

/**
 * @brief 将ipv4分片对象添加到分片链表头部
 *
 * @param frag
 * @param src_ip
 * @param id
 */
static void ipv4_frag_add(ipv4_frag_t *frag, ipaddr_t *src_ip, uint16_t id) {
  // 初始化分片对象
  ipaddr_copy(&frag->src_ip, src_ip);
  frag->tmo = 0;
  frag->id = id;
  nlist_node_init(&frag->node);
  nlist_init(&frag->buf_list);
  // 将分片对象添加到分片链表头部
  nlist_insert_first(&ipv4_frag_list, &frag->node);
}

/**
 * @brief 根据发送方的ip地址和ip包的id字段，在ipv4分片链表中查找到对应的分片对象
 *
 * @param src_ip 发送方ip地址
 * @param id
 * @return ipv4_frag_t*
 */
static ipv4_frag_t *ipv4_frag_find(ipaddr_t *src_ip, uint16_t id) {
  nlist_node_t *node = 0;
  nlist_for_each(node, &ipv4_frag_list) {
    ipv4_frag_t *frag = nlist_entry(node, ipv4_frag_t, node);
    if (ipaddr_is_equal(&frag->src_ip, src_ip) && frag->id == id) {
      // 查找到对应的分片对象，将其位置更新到链表头部(便于下次查询)
      nlist_remove(&ipv4_frag_list, &frag->node);
      nlist_insert_first(&ipv4_frag_list, &frag->node);
      return frag;
    }
  }

  return (ipv4_frag_t *)0;
}

/**
 * @brief 处理被分片的ipv4数据包
 *
 * @param netif
 * @param buf 被分片的数据包
 * @return net_err_t
 */
static net_err_t ipv4_handle_frag(const netif_t *netif, pktbuf_t *buf) {
  net_err_t err = NET_ERR_OK;

  // 获取ipv4数据包
  ipv4_pkt_t *ipv4_pkt = pktbuf_data_ptr(buf);

  // 获取发送方ip地址
  ipaddr_t src_ip;
  ipaddr_from_bytes(&src_ip, ipv4_pkt->hdr.src_ip);

  // 查找对应的分片对象
  ipv4_frag_t *frag = ipv4_frag_find(&src_ip, ipv4_pkt->hdr.id);
  if (frag == (ipv4_frag_t *)0) {
    // 未找到对应的分片对象，分配一个新的分片对象
    frag = ipv4_frag_alloc();
    // 将分片对象添加到分片链表
    ipv4_frag_add(frag, &src_ip, ipv4_pkt->hdr.id);
  }

  return NET_ERR_OK;
}

/**
 * @brief ipv4协议层对接收到的数据包进行处理
 *
 * @param netif 接收到数据包的网络接口
 * @param buf 接收到的数据包
 * @return net_err_t
 */
net_err_t ipv4_recv(const netif_t *netif, pktbuf_t *buf) {
  dbg_info(DBG_IPV4, "recv ipv4 packet....");

  net_err_t err = NET_ERR_OK;

  // 设置数据包头部在内存上的连续性
  err = pktbuf_set_cont(buf, sizeof(ipv4_hdr_t));
  if (err != NET_ERR_OK) {
    dbg_error(DBG_IPV4, "recv ipv4 packet error: pktbuf set cont failed.");
    return err;
  }

  // 获取ipv4数据包
  ipv4_pkt_t *ipv4_pkt = pktbuf_data_ptr(buf);
  if (ipv4_pkt == (ipv4_pkt_t *)0) {
    dbg_error(DBG_IPV4, "recv ipv4 packet error: get ipv4 failed.");
    return NET_ERR_IPV4;
  }

  // 检查ipv4数据包
  err = ipv4_pkt_check(ipv4_pkt, pktbuf_total_size(buf));
  if (err != NET_ERR_OK) {
    dbg_warning(DBG_IPV4, "check failed.");
    return err;
  }

  // 将ipv4数据包头部从网络字节序转换为主机字节序
  ipv4_hdr_ntoh(&ipv4_pkt->hdr);

  // 打印ipv4数据包头部信息
  ipv4_pkt_display(ipv4_pkt);

  // 判断是否为发送给本网络接口的数据包
  ipaddr_t dest_ip;
  ipaddr_from_bytes(&dest_ip, ipv4_pkt->hdr.dest_ip);
  if (!ipaddr_is_match(&dest_ip, &netif->ipaddr,
                       &netif->netmask)) {  // 该数据包不是发给本接口的
    dbg_warning(DBG_IPV4, "ipv4 packet not for me.");
    // TODO: 后续可以添加路由表，进行数据包转发
    return NET_ERR_IPV4;
  }

  // 调整数据包大小，移除尾部的填充字节
  err = pktbuf_resize(buf, ipv4_pkt->hdr.total_len);
  if (err != NET_ERR_OK) {
    dbg_warning(DBG_IPV4, "resize failed.");
    return err;
  }

  if (ipv4_pkt->hdr.frag_more || ipv4_pkt->hdr.frag_offset) {
    // 使用ipv4_handle_frag()函数处理被分片的数据包
    err = ipv4_handle_frag(netif, buf);  //!!! 数据包传递
    if (err != NET_ERR_OK) {
      dbg_error(DBG_IPV4, "handle frag failed.");
      return err;
    }
  } else {
    // 使用ipv4_handle_normal()函数处理普通数据包
    err = ipv4_handle_normal(netif, buf);  //!!! 数据包传递
    if (err != NET_ERR_OK) {
      dbg_error(DBG_IPV4, "handle normal failed.");
      return err;
    }
  }

  dbg_info(DBG_IPV4, "recv ipv4 packet ok.");
  return NET_ERR_OK;
}

/**
 * @brief 将数据包封装成ipv4数据包，并发送
 *
 * @param tran_protocol
 * @param dest_ipaddr
 * @param src_ipaddr
 * @param buf
 * @return net_err_t
 */
net_err_t ipv4_send(uint8_t tran_protocol, const ipaddr_t *dest_ipaddr,
                    const ipaddr_t *src_ipaddr, pktbuf_t *buf) {
  dbg_info(DBG_IPV4, "send an ipv4 packet....");

  net_err_t err = NET_ERR_OK;

  // 给数据包添加ipv4头部
  err = pktbuf_header_add(buf, sizeof(ipv4_hdr_t), PKTBUF_ADD_HEADER_CONT);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_IPV4, "add header failed.");
    return err;
  }

  // 获取ipv4头部对象
  ipv4_pkt_t *pkt = pktbuf_data_ptr(buf);
  // 设置ipv4头部信息
  pkt->hdr.version = IPV4_VERSION;
  ipv4_set_hdr_size(pkt, sizeof(ipv4_hdr_t));
  pkt->hdr.ecn = 0;
  pkt->hdr.diff_service = 0;
  pkt->hdr.total_len = pktbuf_total_size(buf);
  pkt->hdr.id = ipv4_get_id();
  pkt->hdr.flags_frag_offset = 0;
  pkt->hdr.ttl = IPV4_DEFAULT_TTL;
  pkt->hdr.tran_proto = tran_protocol;
  pkt->hdr.hdr_chksum = 0;
  ipaddr_to_bytes(src_ipaddr, pkt->hdr.src_ip);
  ipaddr_to_bytes(dest_ipaddr, pkt->hdr.dest_ip);

  // 打印ipv4数据包头部信息
  ipv4_pkt_display(pkt);

  // 将头部转换为网络字节序
  ipv4_hdr_hton(&pkt->hdr);

  // 计算头部校验和
  pktbuf_acc_reset(buf);  // 重置buf访问位置，从头部开始计算校验和
  pkt->hdr.hdr_chksum = pktbuf_checksum16(buf, ipv4_get_hdr_size(pkt), 0, 1);

  // 通过网络接口发送数据包
  err = netif_send(netif_get_default(), dest_ipaddr, buf);  //!!! 数据包传递
  if (err != NET_ERR_OK) {
    dbg_error(DBG_IPV4, "netif send ip packet failed.");
  }

  return NET_ERR_OK;
}