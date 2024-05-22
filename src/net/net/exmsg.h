#ifndef EXMSG_H
#define EXMSG_H

#include "net_err.h"
#include "netif.h"



typedef struct _exmsg_t {
    double time;
    double delay;
    enum {
        EXMSG_NETIF_IN,
    }type;

    int id;

} exmsg_t;


net_err_t exmsg_module_init(void);
net_err_t exmsg_start(void);
net_err_t exmsg_netif_recv(netif_t *netif);



#endif // EXMSG_H