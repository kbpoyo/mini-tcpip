#ifndef NET_ERR_H
#define NET_ERR_H

typedef enum _net_err_t {
  NET_ERR_UNREACH = 2,
  NET_ERR_NEEDWAIT = 1,
  NET_ERR_OK = 0,
  NET_ERR_SYS = -1,
  NET_ERR_MEM = -2,
  NET_ERR_FULL = -3,
  NET_ERR_TIMEOUT = -4,
  NET_ERR_SIZE = -5,
  NET_ERR_NOSRC = -6,
  NET_ERR_PARAM = -7,
  NET_ERR_STATE = -8,
  NET_ERR_DEV = -9,
  NET_ERR_EXIST = -10,
  NET_ERR_PROTO = -11,
  NET_ERR_NOSUPPORT = -12,
  NET_ERR_PCAP = -13,
  NET_ERR_NETIF = -14,
  NET_ERR_ETHER = -15,
  NET_ERR_ARP = -16,
  NET_ERR_IPV4 = -17,
  NET_ERR_ICMPv4 = -18,
  NET_ERR_EXMSG = -19,
  NET_ERR_SOCKET = -20,
  NET_ERR_SOCKRAW = -21,
  NET_ERR_ROUTE = -22,
  NET_ERR_UDP = -23,
  NET_ERR_PKTBUF = -24,
} net_err_t;


#define Net_Err_Check(err) { \
 if (err != NET_ERR_OK) { \
    return err; \
 }\
}

#endif  // NET_ERR_H