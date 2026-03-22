#include <string.h>

#include "netdev.h"

#include "lwip/sys.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/dhcp.h"
#include "lwip/etharp.h"

#include "netif/ethernet.h"

#include "sys/xdma.h"
#include "sys/xtemac.h"
#include "sys/xtimer.h"

static err_t lwip_ethifInit(struct netif *netif);
static err_t lwip_ethOutput(struct netif *netif, struct pbuf *p);

struct netif ethif;

void _netdev_if(void) {

    lwip_init();

    ip4_addr_t ipaddr, netmask, gw;

    IP4_ADDR(&gw, 0, 0, 0, 0);
    
    IP4_ADDR(&ipaddr, 0, 0, 0, 0);
    IP4_ADDR(&netmask, 0, 0, 0, 0);

    netif_add(&ethif, &ipaddr, &netmask, &gw, NULL, lwip_ethifInit, ethernet_input);

    netif_set_default(&ethif);
    netif_set_up(&ethif);
}

u32_t sys_now(void) {
    return _xtimer_millis();
}

/* Initialize the ethernet interface */
static err_t lwip_ethifInit(struct netif *netif)
{
    strcpy(netif -> name, "e0");

    netif -> hwaddr_len = XTEMAC_MAC_ADDRLEN;
    _xtemac_mac(netif -> hwaddr, netif -> hwaddr_len);

    netif -> mtu = XTEMAC_MTU;
    netif -> flags |= (NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP);

    /* lwIP callback hooks */
    netif -> output = etharp_output;
    netif -> linkoutput = lwip_ethOutput;

    return ERR_OK;
}

/* Send a packet */
static err_t lwip_ethOutput(struct netif *netif, struct pbuf *p)
{
    (void) netif;

    struct pbuf *q;
    for (q = p; q != NULL; q = q -> next) {
        if (_ethdma_txsgenque(q -> payload, q -> len) == NULL) {
            return ERR_IF;
        }
    }

    return ERR_OK;
}