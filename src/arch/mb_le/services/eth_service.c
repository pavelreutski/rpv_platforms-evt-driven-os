#include "command.h"
#include "service.h"

#include "kernel_stdio.h"

#include "sys/xtemac_phy.h"

static void eth_service(void);
static int ethphy_m(const int argc, const char** argv);

_SHELL_COMMAND(ethphy, ethphy_m);
_SERVICE(eth_svc, eth_service);

static void eth_service(void) {
}

static int ethphy_m(const int argc, const char** argv) {
    
    (void) argc;
    (void) argv;

    uint32_t phy_id = _xtemac_phyid();
    _kernel_outStringFormat("phyid - %x\n", phy_id);
    
    return 0;
}