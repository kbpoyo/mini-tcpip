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
#include "route.h"
#include "sock_raw.h"
#include "timer.h"
#include "tools.h"
#include "udp.h"

static ipv4_frag_t ipv4_frag_arr[IPV4_FRAG_MAXCNT];  // ipv4分片数组(分片内存池)
static mblock_t ipv4_frag_mblock;  // ipv4分片内存池管理对象
static nlist_t ipv4_frag_list;  // ipv4分片链表，记录已分配的分片
static net_timer_t ipv4_frag_timer;  // ipv4分片超时定时器

/**
 * @brief 获取分片数据包的有效数据大小
 *
 * @param ipv4_pkt
 * @return int
 */
static int ipv4_frag_data_size(const ipv4_pkt_t *ipv4_pkt) {
  return ipv4_pkt->hdr.total_len - ipv4_get_hdr_size(ipv4_pkt);
}

/**
 * @brief 获取分片数据包的有效内容在原始数据包中的起始地址(即分片的偏移量)
 *
 * @param ipv4_pkt
 * @return int
 */
static int ipv4_frag_start(const ipv4_pkt_t *ipv4_pkt) {
  return ipv4_pkt->hdr.frag_offset * 8;  // 以8字节为单位
}

/**
 * @brief
 * 获取分片数据包的有效内容在原始数据包中的结束地址(即分片偏移量加有效数据长度)
 *
 * @param ipv4_pkt
 * @return int
 */
static int ipv4_frag_end(const ipv4_pkt_t *ipv4_pkt) {
  // 结束地址 = 分片偏移量 + 分片有效数据长度
  return ipv4_frag_start(ipv4_pkt) + ipv4_frag_data_size(ipv4_pkt);
}

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

/**
 * @brief 打印已使用的ipv4分片信息
 *
 */
static void ipv4_frags_show(void) {
  plat_printf("---------------- ipv4 frags ----------------\n");

  nlist_node_t *frag_node = 0;
  int frag_index = 0;

  // 遍历分片链表
  nlist_for_each(frag_node, &ipv4_frag_list) {
    ipv4_frag_t *frag = nlist_entry(frag_node, ipv4_frag_t, node);
    plat_printf("[%d]:\n", frag_index++);
    netif_dum_ip("\tsrc ip: ", &frag->src_ip);
    plat_printf("\n\tid: %d\n", frag->id);
    plat_printf("\ttmo: %d\n", frag->tmo);
    plat_printf("\tbuf cnt: %d\n", nlist_count(&frag->buf_list));

    // 遍历分片对象的数据包链表
    nlist_node_t *buf_node = 0;
    int buf_index = 0;
    plat_printf("\tbufs:\n");
    nlist_for_each(buf_node, &frag->buf_list) {
      pktbuf_t *buf = nlist_entry(buf_node, pktbuf_t, node);
      ipv4_pkt_t *ipv4_pkt = pktbuf_data_ptr(buf);
      plat_printf("\t\t%d:[%d-%d]-(%d)\n", buf_index++,
                  ipv4_frag_start(ipv4_pkt), ipv4_frag_end(ipv4_pkt) - 1,
                  ipv4_frag_data_size(ipv4_pkt));
    }
  }

  plat_printf("--------------------------------------------\n");
}
#else

#define ipv4_pkt_display(ipv4_pkt)
#define ipv4_frags_show()
#endif

static inline int ipv4_get_id(void) {
  static int id = 0;
  return id++;
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
 * @brief ipv4分片超时处理函数
 *
 * @param timer
 * @param arg
 */
static void ipv4_frag_tmo(net_timer_t *timer, void *arg) {
  nlist_node_t *curr_node = 0, *next_node = 0;

  // 遍历分片链表
  for (curr_node = nlist_first(&ipv4_frag_list); curr_node;
       curr_node = next_node) {
    next_node = nlist_node_next(curr_node);

    ipv4_frag_t *frag = nlist_entry(curr_node, ipv4_frag_t, node);
    if (--frag->tmo <= 0) {
      // 分片超时，释放分片对象
      ipv4_frag_free(frag);
    }
  }
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
                  IPV4_FRAG_MAXCNT, NLOCKER_NONE);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_IPV4, "init ipv4 frag mblock failed.");
    return err;
  }

  // 初始化ipv4分片链表
  nlist_init(&ipv4_frag_list);

  // 初始化ipv4分片超时定时器
  err = net_timer_add(&ipv4_frag_timer, "ipv4 frag timer", ipv4_frag_tmo,
                      &ipv4_frag_timer, IPV4_FRAG_SCAN_PERIOD * 1000,
                      NET_TIMER_ACTIVE | NET_TIMER_RELOAD);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_IPV4, "init ipv4 frag timer failed.");
    return err;
  }

  return NET_ERR_OK;
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

  // 初始化路由表
  err = route_init();
  if (err != NET_ERR_OK) {
    dbg_error(DBG_IPV4, "init route table failed.");
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
    uint16_t chksum = tools_checksum16(&ipv4_pkt->hdr, hdr_size, 0, 0, 1);
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

  // 获取ipv4数据包结构, 并提取源ip地址和目的ip地址
  ipv4_pkt_t *pkt = (ipv4_pkt_t *)pktbuf_data_ptr(buf);
  ipaddr_t src_ip, dest_ip;
  ipaddr_from_bytes(&src_ip, pkt->hdr.src_ip);
  ipaddr_from_bytes(&dest_ip, pkt->hdr.dest_ip);

  // 根据数据包的协议字段将其提供给上一层协议处理
  switch (pkt->hdr.tran_proto) {
    case NET_PROTOCOL_ICMPv4: {
      // 当前网络接口已匹配该包的目的地址，直接使用接口地址即可
      err = icmpv4_recv(buf, &netif->ipaddr, &src_ip);  //!!! 数据包传递
      if (err != NET_ERR_OK) {
        dbg_warning(DBG_IPV4, "icmpv4 recv failed.");
        return err;
      }
    } break;

    case NET_PROTOCOL_UDP: {  // UDP协议
      dbg_info(DBG_IPV4, "recv UDP packet.");
      err = udp_recv(buf, &src_ip, &dest_ip); //!!! 数据包传递
      if (err != NET_ERR_OK) {
        dbg_warning(DBG_IPV4, "udp recv failed.");
        if (err == NET_ERR_UNREACH) {
          ipv4_hdr_hton(&pkt->hdr);  // 将头部转换为网络字节序
          icmpv4_make_unreach(&src_ip, &netif->ipaddr,
                                     ICMPv4_CODE_UNREACH_PORT, buf);
        }
        return err;
      }

    } break;

    case NET_PROTOCOL_TCP: {
      dbg_info(DBG_IPV4, "recv TCP packet.");
      ipv4_hdr_hton(&pkt->hdr);  // 将头部转换为网络字节序
      return icmpv4_make_unreach(&src_ip, &netif->ipaddr,
                                 ICMPv4_CODE_UNREACH_PORT, buf);
    } break;

    default: {
      dbg_warning(DBG_IPV4, "unknown transport layer protocol!");
      // 未知的上层协议，直接将ip数据包递交给原始socket模块处理
      // 可通过原始socket模块来进行自定义传输层协议的处理
      err = sockraw_recv_pktbuf(buf);  //!!! 数据包传递
      if (err != NET_ERR_OK) {
        dbg_warning(DBG_IPV4, "sockraw recv failed.");
        return err;
      }

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
  frag->tmo = IPV4_FRAG_TMO / IPV4_FRAG_SCAN_PERIOD;
  frag->id = id;
  nlist_node_init(&frag->node);
  nlist_init(&frag->buf_list);
  // 将分片对象添加到分片链表头部
  nlist_insert_first(&ipv4_frag_list, &frag->node);
}

/**
 * @brief 判断分片对象的数据包链表中是否已接收到所有的分片数据包
 *
 * @param frag
 * @return int
 */
static int ipv4_frag_buf_is_all(ipv4_frag_t *frag) {
  // 遍历分片数据包，判断已接收到的数据包是否连续
  int offset = 0;
  nlist_node_t *node = 0;
  ipv4_pkt_t *curr_pkt = 0;
  nlist_for_each(node, &frag->buf_list) {
    curr_pkt = (ipv4_pkt_t *)pktbuf_data_ptr(nlist_entry(node, pktbuf_t, node));
    int curr_offset = ipv4_frag_start(curr_pkt);
    if (curr_offset != offset) {  // 分片数据包不连续
                                  // 说明还有分片数据包未接收到
      return 0;
    }

    offset += ipv4_frag_data_size(curr_pkt);
  }

  // 分片数据包连续，再根据最后一个分片数据包的标志位判断是否为最后一个分片
  return curr_pkt ? !curr_pkt->hdr.frag_more : 0;
}

/**
 * @brief 将分片对象的数据包链表中的数据包进行重组
 *
 * @param frag
 * @return pktbuf_t*
 */
static pktbuf_t *ipv4_frag_buf_collect(ipv4_frag_t *frag) {
  pktbuf_t *target_buf = 0;  // 重组后的数据包

  // 遍历分片数据包链表，将数据包重组
  nlist_node_t *node = 0;
  while ((node = nlist_remove_first(&frag->buf_list))) {
    pktbuf_t *curr_buf = nlist_entry(node, pktbuf_t, node);  //!!! 获取数据包
    ipv4_pkt_t *curr_pkt = pktbuf_data_ptr(curr_buf);
    if (!target_buf) {
      target_buf = curr_buf;
      continue;
    }

    // 移除数据包的ipv4头部
    net_err_t err = pktbuf_header_remove(curr_buf, ipv4_get_hdr_size(curr_pkt));
    if (err != NET_ERR_OK) {
      dbg_error(DBG_IPV4, "remove head failed.");
      pktbuf_free(curr_buf);    //!!! 释放数据包
      pktbuf_free(target_buf);  //!!! 释放数据包
      return (pktbuf_t *)0;
    }

    // 将数据包的有效数据添加到重组后的数据包中
    err = pktbuf_join(target_buf, curr_buf);  //!!! 数据包合并
    if (err != NET_ERR_OK) {
      dbg_error(DBG_IPV4, "join failed.");
      pktbuf_free(curr_buf);    //!!! 释放数据包
      pktbuf_free(target_buf);  //!!! 释放数据包
      return (pktbuf_t *)0;
    }
  }

  return target_buf;
}

/**
 * @brief 将数据包添加到分片对象的数据包链表
 *
 * @param frag
 * @param frag_buf
 * @return net_err_t
 */
static net_err_t ipv4_frag_buf_add(ipv4_frag_t *frag, pktbuf_t *frag_buf) {
  // 判断分片数据包链表是否已满
  if (nlist_count(&frag->buf_list) >= IPV4_FRAG_BUF_MAXCNT) {
    // 分片数据包链表已满,
    // 直接丢弃已接收到的该分片对应的所有数据包(适用于资源有限的情况)
    dbg_warning(DBG_IPV4, "frag buf list is full.");
    ipv4_frag_free(frag);
    return NET_ERR_IPV4;
  }

  // 获取该ipv4分片数据包的有效数据起始地址在原始数据包中的偏移量
  int buf_start_offset = ipv4_frag_start(pktbuf_data_ptr(frag_buf));

  // 按分片偏移量从小到大的顺序，将数据包添加到分片对象的数据包链表
  nlist_node_t *node = 0;
  nlist_for_each(node, &frag->buf_list) {
    pktbuf_t *buf = nlist_entry(node, pktbuf_t, node);
    ipv4_pkt_t *curr_pkt = pktbuf_data_ptr(buf);
    int curr_start_offset = ipv4_frag_start(curr_pkt);
    if (buf_start_offset == curr_start_offset) {
      // 分片偏移量相同，分片冲突，直接丢弃该分片数据包
      dbg_warning(DBG_IPV4, "frag offset[%d] conflict.", buf_start_offset);
      return NET_ERR_OK;
    } else if (buf_start_offset < curr_start_offset) {
      // 当前数据包的偏移量大于新数据包的偏移量
      // 将新数据包插入到当前数据包之前
      nlist_insert_before(&frag->buf_list, node, &frag_buf->node);
      return NET_ERR_OK;
    }
  }

  // 数据包链表为空或者新数据包的起始地址偏移量最大，将新数据包添加到链表尾部
  nlist_insert_last(&frag->buf_list, &frag_buf->node);
  return NET_ERR_OK;
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

  // 将数据包添加到分片对象的数据包链表
  err = ipv4_frag_buf_add(
      frag, buf);  //!!! 数据包转交, 数据包转交成功后，将不可向上层返回错误
  if (err != NET_ERR_OK) {
    dbg_error(DBG_IPV4, "add frag buf failed.");
    return err;
  }

  // 打印系统此时所有的分片信息
  ipv4_frags_show();

  // 判断分片对象的数据包链表中是否已接收到所有的分片数据包
  if (ipv4_frag_buf_is_all(frag)) {  // 接收到所有的分片数据包
    pktbuf_t *target_buf = ipv4_frag_buf_collect(frag);  //!!! 获取数据包
    if (target_buf) {  // 数据包重组成功
      // 对重组后的数据包进行处理
      err = ipv4_handle_normal(netif, target_buf);  //!!! 数据包传递
      if (err != NET_ERR_OK) {
        dbg_error(DBG_IPV4, "handle normal failed.");
        pktbuf_free(target_buf);  //!!! 释放数据包
      }
    } else {  // 数据包重组失败
      dbg_error(DBG_IPV4, "collect failed.");
    }

    // 释放分片对象
    ipv4_frag_free(frag);
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
 * @brief 对上层数据包进行分片处理，并包装成ipv4数据包发送
 *
 * @param tran_protocol 上层协议类型
 * @param dest_ipaddr 目的ip地址
 * @param src_ipaddr 源ip地址
 * @param buf 上层数据包(不包含ipv4头部)
 * @param netif 发送数据包的网络接口
 * @param next_hop 下一跳地址
 * @return net_err_t
 */
net_err_t ipv4_frag_send(uint8_t tran_protocol, const ipaddr_t *dest_ipaddr,
                         const ipaddr_t *src_ipaddr, pktbuf_t *buf,
                         netif_t *netif, const ipaddr_t *next_hop) {
  // TODO:
  // 后续优化思路：在进行分片处理时，不进行内存的再分配与拷贝，而是直接在原数据包上进行分片处理
  dbg_info(DBG_IPV4, "send an ipv4 frag packet....");
  net_err_t err = NET_ERR_OK;

  // 获取当前要发送的ipv4所有分片
  int ipv4_id = ipv4_get_id();

  // 对数据包进行分片处理
  int offset = 0, total_size = pktbuf_total_size(buf);
  pktbuf_acc_reset(buf);  // 重置待发送数据的访问位置
  while (total_size > 0) {
    // 计算当前分片的有效数据长度
    int curr_size = total_size > netif->mtu ? (netif->mtu - sizeof(ipv4_hdr_t))
                                            : total_size;
    // 为当前分片数据包分配内存, 大小 = 有效数据长度(curr_size) + ipv4头部大小
    pktbuf_t *frag_buf =
        pktbuf_alloc(curr_size + sizeof(ipv4_hdr_t));  //!!! 分配数据包
    if (frag_buf == (pktbuf_t *)0) {
      dbg_error(DBG_IPV4, "alloc frag buf failed.");
      return NET_ERR_IPV4;
    }

    // 从待发送数据包里拷贝curr_size大小的数据到分片中
    pktbuf_seek(frag_buf, sizeof(ipv4_hdr_t));
    err = pktbuf_copy(frag_buf, buf, curr_size);
    if (err != NET_ERR_OK) {
      dbg_error(DBG_IPV4, "pktbuf copy failed.");
      pktbuf_free(frag_buf);  // 释放数据包
      return err;
    }

    // 移除待发送数据包的curr_size大小的数据,防止内存占用过大
    pktbuf_header_remove(buf, curr_size);

    // 设置数据包头部在内存上的连续性
    err = pktbuf_set_cont(frag_buf, sizeof(ipv4_hdr_t));
    if (err != NET_ERR_OK) {
      dbg_error(DBG_IPV4, "set cont failed.");
      pktbuf_free(frag_buf);  //!!! 释放数据包
      return err;
    }

    // 设置分片的ipv4头部
    ipv4_pkt_t *pkt = pktbuf_data_ptr(frag_buf);
    pkt->hdr.version = IPV4_VERSION;
    ipv4_set_hdr_size(pkt, sizeof(ipv4_hdr_t));
    pkt->hdr.ecn = 0;
    pkt->hdr.diff_service = 0;
    pkt->hdr.total_len = pktbuf_total_size(frag_buf);
    pkt->hdr.id = ipv4_id;
    pkt->hdr.frag_offset = offset >> 3;  // 偏移量以8个字节为单位
    pkt->hdr.frag_more =
        total_size >
        curr_size;  // 若当前分片的大小小于待发送的数据的总大小，则后续还有分片
    pkt->hdr.frag_disable = 0;
    pkt->hdr.frag_reserved = 0;
    pkt->hdr.ttl = IPV4_DEFAULT_TTL;
    pkt->hdr.tran_proto = tran_protocol;
    pkt->hdr.hdr_chksum = 0;
    if (!src_ipaddr || ipaddr_is_any(src_ipaddr)) {
      // 源ip地址为空则使用网络接口的ip地址
      ipaddr_to_bytes(&netif->ipaddr, pkt->hdr.src_ip);
    } else {
      ipaddr_to_bytes(src_ipaddr, pkt->hdr.src_ip);
    }
    ipaddr_to_bytes(dest_ipaddr, pkt->hdr.dest_ip);

    // 打印ipv4数据包头部信息
    ipv4_pkt_display(pkt);

    // 将头部转换为网络字节序
    ipv4_hdr_hton(&pkt->hdr);

    // 计算头部校验和
    pktbuf_acc_reset(frag_buf);  // 重置buf访问位置，从头部开始计算校验和
    pkt->hdr.hdr_chksum =
        pktbuf_checksum16(frag_buf, ipv4_get_hdr_size(pkt), 0, 1);

    // 通过网络接口将数据包发送到下一跳目标地址
    err = netif_send(netif, next_hop, frag_buf);  //!!! 数据包传递
    if (err != NET_ERR_OK) {
      dbg_error(DBG_IPV4, "netif send ip packet failed.");
      pktbuf_free(frag_buf);  //!!! 释放数据包
      return err;
    }

    // 更新待发送数据大小和分片偏移量
    offset += curr_size;
    total_size -= curr_size;
  }

  // 数据包处理完毕，释放数据包
  pktbuf_free(buf);  //!!! 释放数据包

  return NET_ERR_OK;
}

/**
 * @brief 将数据包封装成ipv4数据包，并发送
 *
 * @param tran_protocol 上层协议类型
 * @param dest_ipaddr 目的ip地址
 * @param src_ipaddr 源ip地址
 * @param buf 上层数据包
 * @return net_err_t
 */
net_err_t ipv4_send(uint8_t tran_protocol, const ipaddr_t *dest_ipaddr,
                    const ipaddr_t *src_ipaddr, pktbuf_t *buf) {
  dbg_info(DBG_IPV4, "send an ipv4 packet....");
  net_err_t err = NET_ERR_OK;

  // 根据目的ip地址查找路由表，获取下一跳的ip地址和发送该包的网络接口
  route_entry_t *rt_entry = route_find(dest_ipaddr);
  if (!rt_entry) {
    dbg_error(DBG_IPV4, "route entry not found.");
    return NET_ERR_IPV4;
  }
  netif_t *netif = rt_entry->netif;
  const ipaddr_t *next_hop;
  if (ipaddr_is_any(&rt_entry->next_hop)) {
    // 下一跳地址为空, 即目的地址在链路上，直接使用目的地址即可
    next_hop = dest_ipaddr;
  } else {  // 下一跳地址不为空，使用下一跳地址，让下一跳路由该数据包
    next_hop = &rt_entry->next_hop;
  }

  // 判断数据包是否需要进行分片处理
  int ipv4_total_size = pktbuf_total_size(buf) + sizeof(ipv4_hdr_t);
  if (netif->mtu && ipv4_total_size > netif->mtu) {
    // 数据包大小超过了网络接口链路层的最大传输单元
    // 需要对数据包进行分片处理
    err = ipv4_frag_send(tran_protocol, dest_ipaddr, src_ipaddr, buf, netif,
                         next_hop);  //!!! 数据包传递
    if (err != NET_ERR_OK) {
      dbg_error(DBG_IPV4, "frag send failed.");
      return err;
    }

    return NET_ERR_OK;
  }

  // 数据包不需要进行分片处理, 给数据包添加ipv4头部
  err = pktbuf_header_add(buf, sizeof(ipv4_hdr_t), PKTBUF_ADD_HEADER_CONT);
  if (err != NET_ERR_OK) {
    dbg_error(DBG_IPV4, "add header failed.");
    return err;
  }

  // 获取ipv4头部对象, 并设置ipv4头部信息
  ipv4_pkt_t *pkt = pktbuf_data_ptr(buf);
  pkt->hdr.version = IPV4_VERSION;
  ipv4_set_hdr_size(pkt, sizeof(ipv4_hdr_t));
  pkt->hdr.ecn = 0;
  pkt->hdr.diff_service = 0;
  pkt->hdr.total_len = ipv4_total_size;
  pkt->hdr.id = ipv4_get_id();
  pkt->hdr.flags_frag_offset = 0;
  pkt->hdr.ttl = IPV4_DEFAULT_TTL;
  pkt->hdr.tran_proto = tran_protocol;
  pkt->hdr.hdr_chksum = 0;
  if (!src_ipaddr || ipaddr_is_any(src_ipaddr)) {
    // 源ip地址为空则使用网络接口的ip地址
    ipaddr_to_bytes(&netif->ipaddr, pkt->hdr.src_ip);
  } else {
    ipaddr_to_bytes(src_ipaddr, pkt->hdr.src_ip);
  }
  ipaddr_to_bytes(dest_ipaddr, pkt->hdr.dest_ip);

  // 打印ipv4数据包头部信息
  ipv4_pkt_display(pkt);

  // 将头部转换为网络字节序
  ipv4_hdr_hton(&pkt->hdr);

  // 计算头部校验和
  pktbuf_acc_reset(buf);  // 重置buf访问位置，从头部开始计算校验和
  pkt->hdr.hdr_chksum = pktbuf_checksum16(buf, ipv4_get_hdr_size(pkt), 0, 1);

  // 通过网络接口将数据包发送到下一跳目标地址
  err = netif_send(netif, next_hop, buf);  //!!! 数据包传递
  if (err != NET_ERR_OK) {
    dbg_error(DBG_IPV4, "netif send ip packet failed.");
    return err;
  }

  return NET_ERR_OK;
}