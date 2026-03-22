#include <string.h>
#include <stdlib.h>

#include "eth_c.h"

#include "monitor.h"
#include "command.h"
#include "service.h"

#include "kernel_jrnl.h"

#include "kernel_stdio.h"
#include "kernel_signal.h"

#include "sys/xdma.h"
#include "sys/xtemac.h"
#include "sys/xtemac_phy.h"

#include "lwip/sys.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/dhcp.h"
#include "lwip/etharp.h"
#include "lwip/timeouts.h"

#include "netif/ethernet.h"

enum ethcon_status_u : uint8_t {
    
    ETH_LINKUP,
    ETH_LINKDOWN
};

static phylink_t phylink = { 0 };
static uint8_t link_reg = ETH_LINKDOWN;

static void eth_linkup(struct netif *netif);
static void eth_linkdown(struct netif *netif);

static void lwip_ethInput(struct netif *netif);

/********************************** ethernet service & commands *****************************************/

static void eth_service(void);

static int dhcp_m(const int argc, const char** argv);
static int ethmac_m(const int argc, const char** argv);
static int ethphy_m(const int argc, const char** argv);
static int ethlink_m(const int argc, const char** argv);
static int ethstat_m(const int argc, const char** argv);

_SHELL_COMMAND(ethmac, ethmac_m);
_SHELL_COMMAND(ethphy, ethphy_m);
_SHELL_COMMAND(ethlink, ethlink_m);
_SHELL_COMMAND(ethstat, ethstat_m);

_SHELL_COMMAND(dhcp, dhcp_m);

_SERVICE(eth_svc, eth_service);

static void eth_service(void) {

    /* lookup for inbound traffic - it shall empty rx queue */
    lwip_ethInput(netif_default);

    /* handle TCP, ICMP or DHCP timers */
    sys_check_timeouts();

    /* flush eth tx queue */
    _ethdma_txsgflush();

    sigset_t sgls;

    _kernel_sigemptyset(&sgls);

    _kernel_sigaddset(&sgls, SIGINT);
    _kernel_sigaddset(&sgls, SIGBUS);

    _kernel_sigprocmask(SIG_BLOCK, &sgls, NULL);

    _kernel_sigpending(&sgls);

    if (!_kernel_sigismember(&sgls, SIGINT) &&
            !_kernel_sigismember(&sgls, SIGBUS)) { // SIGINT nor SIGBUS ?
        return;
    }

    /* poll link or rx/tx bus error signals after SIGINT or SIGBUS */
    bool eth_sgl = _xtemac_phylinkSignal() || 
                    (_ethdma_txbuserrSignal() || _ethdma_rxbuserrSignal());
    
    if (eth_sgl && _xtemac_phylinkSignal()) {

        link_reg = 
            _xtemac_phylink(&phylink) ? ETH_LINKUP : ETH_LINKDOWN;

        switch (link_reg) {

            case ETH_LINKUP: { eth_linkup(netif_default); } break;
            case ETH_LINKDOWN: { eth_linkdown(netif_default); } break;
        
            default: break;
        }
    }

    if (eth_sgl && (_ethdma_rxbuserrSignal() || _ethdma_txbuserrSignal())) {

        _kernel_jentry("eth DMA internal error!");
        _xtemac_recover(); /* recover ethernet transceiver */
    }

    if (eth_sgl) {
        _kernel_sigprocmask(SIG_UNBLOCK, &sgls, NULL);
    }
}

static void eth_linkup(struct netif *netif) {

    _xtemac_trxenable();

    if (netif != NULL) {
        netif_set_link_up(netif);
    }   
}

static void eth_linkdown(struct netif *netif) {

    _xtemac_trxdisable();

    if (netif != NULL) {
        netif_set_link_down(netif);
    }
}

/********************************** lwIP glue ***************************************************/

static void lwip_ethInput(struct netif *netif) {

    if (netif == NULL || 
            (!netif_is_up(netif) || !netif_is_link_up(netif))) {
        return;
    }

    size_t rx_len;
    uint8_t rxp[XTEMAC_MTU];

    volatile bool lwipAlloc_error = false;
    volatile bool lwipInput_error = false; 

    eth_counter_t rx_qcount = eth_rxqcount();

    while((rx_qcount--) && 
            _ethdma_rxsgcmplt(rxp, sizeof(rxp), &rx_len) != NULL) {

        struct pbuf *p = pbuf_alloc(PBUF_RAW, rx_len, PBUF_POOL);

        if (p == NULL) {

            lwipAlloc_error = true;
            continue;
        }

        pbuf_take(p, rxp, rx_len);

        if (netif -> input(p, netif) != ERR_OK) {

            pbuf_free(p);
            lwipInput_error = true;
        }
    }

    if (lwipInput_error) {
        _kernel_jentry("lwIP input packet processing error");
    }

    if (lwipAlloc_error) {
        _kernel_jentry("lwIP cant allocate the buffer for input packet");
    }
}

/********************************** ethernet commands *******************************************/

static int dhcp_m(const int argc, const char** argv) {

    (void) argc;
    (void) argv;

    if (netif_default == NULL) {

        _kernel_outString("no default net interface exist\n");
        return -1;
    }

    struct netif *netif = netif_default;

    if (!netif_is_link_up(netif)) {

        _kernel_outString("default net interface link is down\n");
        return 0;
    }

    if (!ip_addr_isany_val(netif -> ip_addr)) {

        _kernel_outStringFormat("IP: %s\n", ipaddr_ntoa(&netif -> ip_addr));
        _kernel_outStringFormat("Gateway: %s\n", ipaddr_ntoa(&netif -> gw));
        _kernel_outStringFormat("Netmask: %s\n", ipaddr_ntoa(&netif -> netmask));

        return 0;
    }

    _kernel_outString("acquire ip address from dhcp\n");
    dhcp_start(netif);

    return 0;
}

static int ethmac_m(const int argc, const char** argv) {

    (void) argc;
    (void) argv;

    uint8_t m_addr[6];
    _xtemac_mac(m_addr, sizeof(m_addr));
    
    _kernel_outStringFormat(
        "MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n", 
            (unsigned)m_addr[0], (unsigned)m_addr[1], (unsigned)m_addr[2], 
            (unsigned)m_addr[3], (unsigned)m_addr[4], (unsigned)m_addr[5]);

    return 0;
}

static int ethstat_m(const int argc, const char** argv) {

    (void) argc;
    (void) argv;

    eth_counter_t rx_c = eth_rxcount();
    eth_counter_t tx_c = eth_txcount();
    eth_counter_t lost_c = eth_lostcount();

    eth_counter_t rx_qhead = eth_rxqhead();
    eth_counter_t rx_qtail = eth_rxqtail();

    eth_counter_t tx_qhead = eth_txqhead();
    eth_counter_t tx_qtail = eth_txqtail();

    if (rx_c == 0) {

        _kernel_outString("no ethernet frames received\n"); 
        return 0;       
    }

    _kernel_outString("ethernet stat\n");

    _kernel_outStringFormat("rx queue: %d head, %d tail\n", (int)rx_qhead, (int)rx_qtail);
    _kernel_outStringFormat("tx queue: %d head, %d tail\n", (int)tx_qhead, (int)tx_qtail);
    _kernel_outStringFormat("packet: %d received, %d transmitted, %d lost\n", (int)rx_c, (int)tx_c, (int)lost_c);
    
    return 0;
}

static int ethlink_m(const int argc, const char** argv) {

    (void) argc;
    (void) argv;

    switch (link_reg) {

        case ETH_LINKUP: {

            _kernel_outString("link up\n");

            _kernel_outStringFormat("speed: %s\n", 
                phylink.speed == SPEED_10MBPS ? "10 Mbps" : "100 Mbps");
            _kernel_outStringFormat("duplex: %s\n",
                phylink.link == LINK_HALF_DUPLEX ? "half" : "full");            
        } break;

        case ETH_LINKDOWN: {
            _kernel_outString("cable not plugged or link down\n");
        } break;
    
        default:
            break;
    }

    return 0;
}

static int ethphy_m(const int argc, const char** argv) {
    
    (void) argc;
    (void) argv;

    uint32_t phy_id = _xtemac_phyid();

    if (phy_id == 0) {

        _kernel_outString("eth PHY not detected.\n");
        return -1;
    }

    uint16_t phyid_hi = (phy_id >> 16);
    uint16_t phyid_low = (phy_id & 0xFFFF);

    _kernel_outString("eth PHY detected\n");
    _kernel_outStringFormat("PHYid %04x:%04x\n", phyid_hi, phyid_low);

    return 0;
}