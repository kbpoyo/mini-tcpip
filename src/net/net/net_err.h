#ifndef NET_ERR_H
#define NET_ERR_H

typedef enum _net_err_t {
  NET_ERR_OK = 0,
  NET_ERR_SYS = -1,
  NET_ERR_MEM = -2,
  NET_ERR_FIXQ_FULL = -3,
  NET_ERR_TIMEOUT = -4,
} net_err_t;

#endif  // NET_ERR_H