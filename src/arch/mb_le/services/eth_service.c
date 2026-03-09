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

enum ethcon_status_u : uint8_t {
    
    ETH_LINKUP,
    ETH_LINKDOWN
};

static uint8_t lastp[1536];
static size_t lastpsize;

static phylink_t phylink = { 0 };
static uint8_t link_reg = ETH_LINKDOWN;

static void eth_service(void);

static int ethpack_m(const int argc, const char** argv);
static int ethphy_m(const int argc, const char** argv);
static int ethlink_m(const int argc, const char** argv);
static int ethstat_m(const int argc, const char** argv);

_SHELL_COMMAND(ethphy, ethphy_m);
_SHELL_COMMAND(ethtail, ethpack_m);
_SHELL_COMMAND(ethlink, ethlink_m);
_SHELL_COMMAND(ethstat, ethstat_m);

_SERVICE(eth_svc, eth_service);

static void eth_service(void) {

    /* lookup for inbound traffic - it shall empty rx queue */
    _ethdma_rxsgcmplt(lastp, sizeof(lastp), &lastpsize);

    sigset_t sigint;

    _kernel_sigemptyset(&sigint);
    _kernel_sigaddset(&sigint, SIGINT);
    _kernel_sigprocmask(SIG_BLOCK, &sigint, NULL);

    _kernel_sigpending(&sigint);

    if (!_kernel_sigismember(&sigint, SIGINT)) {
        return;
    }

    /* poll link or rx complete signals after SIGINT */
    bool eth_sgl = _xtemac_phylinkSignal() || _ethdma_rxbuserrSignal();
    
    if (eth_sgl && _xtemac_phylinkSignal()) {

        link_reg = 
            _xtemac_phylink(&phylink) ? ETH_LINKUP : ETH_LINKDOWN;

        switch (link_reg) {

            case ETH_LINKUP: { _xtemac_trxenable(); } break;
            case ETH_LINKDOWN: { _xtemac_trxdisable(); } break;
        
            default: break;
        }
    }

    if (eth_sgl && _ethdma_rxbuserrSignal()) {

        _xtemac_trxdisable();
        _xtemac_trxenable();
    }

    if (eth_sgl) {
        _kernel_sigprocmask(SIG_UNBLOCK, &sigint, NULL);
    }
}

static int ethpack_m(const int argc, const char** argv) {

    (void) argc;
    (void) argv;

    if (eth_rxcount() == 0) {

        _kernel_outString("tail no frames received\n");
        return 0;
    }

    _kernel_outStringFormat("tail ethernet %d bytes packet stat:\n", (int)lastpsize);

    _kernel_outStringFormat("type: %02x\n", (unsigned)((lastp[12] << 8) | lastp[13]));

    _kernel_outStringFormat("source MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", (unsigned)lastp[6], (unsigned)lastp[7], (unsigned)lastp[8], (unsigned)lastp[9], (unsigned)lastp[10], (unsigned)lastp[11]);
    _kernel_outStringFormat("destination MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", (unsigned)lastp[0], (unsigned)lastp[1], (unsigned)lastp[2], (unsigned)lastp[3], (unsigned)lastp[4], (unsigned)lastp[5]);

    hex_monitor(lastp, lastpsize);

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