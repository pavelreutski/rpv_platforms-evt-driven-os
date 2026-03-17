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

enum ethcon_status_u : uint8_t {
    
    ETH_LINKUP,
    ETH_LINKDOWN
};

static uint8_t lastp[1536];
static size_t lastpsize;

static bool arp_reply;

static uint8_t arp_ip[4];
static uint8_t arp_mac[6];

static phylink_t phylink = { 0 };
static uint8_t link_reg = ETH_LINKDOWN;

static void eth_service(void);

static int etharp_m(const int argc, const char** argv);
static int ethmac_m(const int argc, const char** argv);
static int ethpack_m(const int argc, const char** argv);
static int ethphy_m(const int argc, const char** argv);
static int ethlink_m(const int argc, const char** argv);
static int ethstat_m(const int argc, const char** argv);

_SHELL_COMMAND(ethmac, ethmac_m);
_SHELL_COMMAND(ethphy, ethphy_m);
_SHELL_COMMAND(etharp, etharp_m);
_SHELL_COMMAND(ethtail, ethpack_m);
_SHELL_COMMAND(ethlink, ethlink_m);
_SHELL_COMMAND(ethstat, ethstat_m);

_SERVICE(eth_svc, eth_service);

static void eth_service(void) {

    /* flush eth tx queue */
    _ethdma_txsgflush();

    /* lookup for inbound traffic - it shall empty rx queue */
    _ethdma_rxsgcmplt(lastp, sizeof(lastp), &lastpsize);

    uint8_t t_ip[] = { 192, 168, 0, 10 };
    if ((lastp[0x0c] == 0x08) && (lastp[0x0d] == 0x06) && (lastp[0x14] == 0x00) && (lastp[0x15] = 0x02) && memcmp(&lastp[0x26], t_ip, sizeof(t_ip)) == 0) {

        arp_reply = true;

        memcpy(arp_ip, &lastp[0x1c], sizeof(arp_ip));
        memcpy(arp_mac, &lastp[0x16], sizeof(arp_mac));
    }

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

            case ETH_LINKUP: { _xtemac_trxenable(); } break;
            case ETH_LINKDOWN: { _xtemac_trxdisable(); } break;
        
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

static int etharp_m(const int argc, const char** argv) {

    (void) argc;
    (void) argv;

    uint8_t arp[] = {

        /* Ethernet Header */
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  // Destination MAC (Broadcast)
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,  // Source MAC (example)
        0x08, 0x06,                          // EtherType (ARP)

        /* ARP Header */
        0x00, 0x01,  // Hardware type (Ethernet)
        0x08, 0x00,  // Protocol type (IPv4)
        0x06,        // Hardware size
        0x04,        // Protocol size
        0x00, 0x01,  // Opcode (request)

        /* Sender MAC */
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,

        /* Sender IP (192.168.0.10 example) */
        0xc0, 0xa8, 0x00, 0x0a,

        /* Target MAC (unknown) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        /* Target IP (192.168.0.116) */
        0xc0, 0xa8, 0x00, 0x74
    };

    _xtemac_mac(&arp[6], 6);  /* set source MAC */
    _xtemac_mac(&arp[22], 6); /* set sender MAC */ 

    if (_ethdma_txsgenque(arp, sizeof(arp)) == NULL) {

        _kernel_outString("ethernet tx queue full or locked (tx queue flush ongoing)\n");
        return 0;
    }

    _kernel_outString("ARP request put on tx queue\n");
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

static int ethpack_m(const int argc, const char** argv) {

    (void) argc;
    (void) argv;

    if (eth_rxcount() == 0) {

        _kernel_outString("tail no frames received\n");
        return 0;
    }

    if (arp_reply) {

        arp_reply = false;

        _kernel_outString("ARP request replied\n");

        _kernel_outStringFormat(
            "reply IPv4: %d.%d.%d.%d\n", 
                (int)arp_ip[0], (int)arp_ip[1], (int)arp_ip[2], (int)arp_ip[3]);

        _kernel_outStringFormat(
            "reply MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", 
                (unsigned)arp_mac[0], (unsigned)arp_mac[1], (unsigned)arp_mac[2], 
                (unsigned)arp_mac[3], (unsigned)arp_mac[4], (unsigned)arp_mac[5]);

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