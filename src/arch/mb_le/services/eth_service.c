#include <stdlib.h>

#include "command.h"
#include "service.h"

#include "kernel_jrnl.h"

#include "kernel_stdio.h"
#include "kernel_signal.h"

#include "sys/xtemac_phy.h"

enum ethcon_status_u : uint8_t {
    
    ETH_LINKUP,
    ETH_LINKDOWN
};

static phylink_t phylink = { 0 };
static uint8_t link_reg = ETH_LINKDOWN;

static void eth_service(void);

static int ethphy_m(const int argc, const char** argv);
static int ethlink_m(const int argc, const char** argv);

_SHELL_COMMAND(ethphy, ethphy_m);
_SHELL_COMMAND(ethlink, ethlink_m);

_SERVICE(eth_svc, eth_service);

static void eth_service(void) {

    sigset_t sigint;

    _kernel_sigemptyset(&sigint);
    _kernel_sigaddset(&sigint, SIGINT);
    _kernel_sigprocmask(SIG_BLOCK, &sigint, NULL);

    _kernel_sigpending(&sigint);

    if (!_kernel_sigismember(&sigint, SIGINT) || !_xtemac_phylinkSignal()) {
        return;
    }

    link_reg = 
        _xtemac_phylink(&phylink) ? ETH_LINKUP : ETH_LINKDOWN;

    _kernel_sigprocmask(SIG_UNBLOCK, &sigint, NULL);
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