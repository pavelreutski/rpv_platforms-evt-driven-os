#include "netdev.h"

#include "lwip/sys.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/dhcp.h"
#include "lwip/etharp.h"

#include "netif/ethernet.h"

static err_t ethernetif_init(struct netif *netif);

/* Forward declarations */
static void low_level_init(struct netif *netif);
static err_t low_level_output(struct netif *netif, struct pbuf *p);
static struct pbuf* low_level_input(struct netif *netif);

struct netif ethif;

void _netdev_if(void) {

    lwip_init();

    ip4_addr_t ipaddr, netmask, gw;

    IP4_ADDR(&ipaddr, 0, 0, 0, 0);
    IP4_ADDR(&netmask, 0, 0, 0, 0);
    IP4_ADDR(&gw, 0, 0, 0, 0);

    netif_add(&ethif, &ipaddr, &netmask, &gw, NULL, ethernetif_init, ethernet_input);

    netif_set_default(&ethif);
    netif_set_up(&ethif);

    dhcp_start(&ethif);
}

u32_t sys_now(void) {
    return 0;
}

/* Initialize the ethernet interface */
static err_t ethernetif_init(struct netif *netif)
{
    netif->name[0] = 'e';
    netif->name[1] = '0';

    netif->hwaddr_len = 6;

    netif->hwaddr[0] = 0x02;
    netif->hwaddr[1] = 0x12;
    netif->hwaddr[2] = 0x34;
    netif->hwaddr[3] = 0x56;
    netif->hwaddr[4] = 0x78;
    netif->hwaddr[5] = 0x9A;

    /* lwIP callback hooks */
    netif->output = etharp_output;
    netif->linkoutput = low_level_output;

    /* Hardware initialization */
    low_level_init(netif);

    return ERR_OK;
}

/* Low-level hardware initialization */
static void low_level_init(struct netif *netif)
{
    /* TODO: Initialize your MAC hardware here */

    netif->mtu = 1500;
    netif->flags |= (NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP);

    /* Optionally initialize link state or hardware buffers */
}

/* Send a packet */
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    (void) netif;

    /* TODO: Copy pbuf contents to your hardware transmit buffer and trigger TX */

    struct pbuf *q;
    for (q = p; q != NULL; q = q->next) {
        /* Example: send q->payload of length q->len to your MAC */
        (void)q->payload;
        (void)q->len;
    }

    /* Indicate success */
    return ERR_OK;
}

/* Receive a packet and allocate pbuf */
static struct pbuf* low_level_input(struct netif *netif)
{
    (void) netif;

    /* TODO: Read a packet from your MAC into a temporary buffer */

    /* Return NULL if no packet is available */
    return NULL;
}

/* Poll function to call in main loop */
void ethernetif_input(struct netif *netif)
{
    struct pbuf *p;
    while ((p = low_level_input(netif)) != NULL) {
        if (netif->input(p, netif) != ERR_OK) {
            pbuf_free(p);
        }
    }
}