#include <stdint.h>

#include "sys/xdma.h"
#include "sys/xtemac.h"
#include "sys/xtemac_phy.h"

#define XTEMAC_BASE                     (0x44A60000)

#define XTEMAC_REG_BASE                 (XTEMAC_BASE + 0x400)
#define XTEMAC_MAC_BASE                 (XTEMAC_BASE + 0x700)

#define XTEMAC                          ((xtemac_t *) XTEMAC_REG_BASE)
#define XTEMAC_MAC                      ((xtemac_mac_t *) XTEMAC_MAC_BASE)

#define XTEMAC_MAX_MAC                  (6)
#define XTEMAC_MAX_QUEUE                (10)
#define XTEMAC_MAX_FRAME                (1536)

#define XTEMAC_IO_BUFFER                (XTEMAC_MAX_FRAME * XTEMAC_MAX_QUEUE)

#define XTEMAC_DEF_MACADDR              { 0x02, 0x1A, 0x11, 0x00, 0x7C, 0x55 }

enum xtemac_speed_u : uint8_t {

    XTEMAC_10MBPS_SPEED  = 0x00,
    XTEMAC_100MPBS_SPEED = 0x01,
    XTEMAC_1GBPS_SPEED   = 0x02
};

/********************************* XTEMAC registers *************************************/

/* XTEMAC rx config word 1 register */

typedef union xtemac_rx_cfg_word1_u {

    uint32_t reg;

    struct {

        uint32_t pause_mac_addr_high  : 16; /* Bits 0–15  : R/W Upper 16 bits of Pause Frame MAC SA [47:32] */
        uint32_t reserved16_23        : 8;  /* Bits 16–23 : RO  Reserved */
        uint32_t ctrl_len_chk_dis     : 1;  /* Bit 24     : R/W Control frame length check disable (1 = disable check) */
        uint32_t len_type_chk_dis     : 1;  /* Bit 25     : R/W Length/Type field check disable (1 = disable check) */
        uint32_t half_duplex          : 1;  /* Bit 26     : R/W Duplex mode (1 = half, 0 = full) */
        uint32_t vlan_enable          : 1;  /* Bit 27     : R/W VLAN frame enable (1 = accept VLAN frames) */
        uint32_t receiver_enable      : 1;  /* Bit 28     : R/W Receiver enable (1 = enable RX block) */
        uint32_t inband_fcs_enable    : 1;  /* Bit 29     : R/W In-band FCS enable (1 = pass FCS to client) */
        uint32_t jumbo_frame_enable   : 1;  /* Bit 30     : R/W Jumbo frame enable (1 = allow oversized frames) */
        uint32_t reset                : 1;  /* Bit 31     : R/W Receiver reset (self-clearing) */ 
    };
} xtemac_rx_cfg_w1_t;

/* XTEMAC tx config word register */

typedef union xtemac_tx_cfg_word_u {
    
    uint32_t reg;

    struct {

        uint32_t reserved0_24        : 25; /* Bits 0–24  : RO  Reserved */
        uint32_t ifg_adjust_enable   : 1;  /* Bit 25     : R/W Interframe Gap adjust enable (1 = use tx_ifg_delay input) */
        uint32_t half_duplex         : 1;  /* Bit 26     : R/W Duplex mode (1 = half, 0 = full) */
        uint32_t vlan_enable         : 1;  /* Bit 27     : R/W VLAN frame recognition enable */
        uint32_t transmit_enable     : 1;  /* Bit 28     : R/W Transmitter enable (1 = enable TX block) */
        uint32_t inband_fcs_enable   : 1;  /* Bit 29     : R/W In-band FCS enable (1 = client supplies FCS) */
        uint32_t jumbo_frame_enable  : 1;  /* Bit 30     : R/W Jumbo frame enable (1 = allow oversized frames) */
        uint32_t reset               : 1;  /* Bit 31     : R/W Transmitter reset (self-clearing) */
    };
} xtemax_tx_cfg_t;

/*XTEMAC flow control config word register */

typedef union xtemac_flow_ctl_cfg_word_u {

    uint32_t reg;

    struct {

        uint32_t rx_pfc_p0_en        : 1;  /* Bit 0  : R/W RX Priority 0 PFC enable */
        uint32_t rx_pfc_p1_en        : 1;  /* Bit 1  : R/W RX Priority 1 PFC enable */
        uint32_t rx_pfc_p2_en        : 1;  /* Bit 2  : R/W RX Priority 2 PFC enable */
        uint32_t rx_pfc_p3_en        : 1;  /* Bit 3  : R/W RX Priority 3 PFC enable */
        uint32_t rx_pfc_p4_en        : 1;  /* Bit 4  : R/W RX Priority 4 PFC enable */
        uint32_t rx_pfc_p5_en        : 1;  /* Bit 5  : R/W RX Priority 5 PFC enable */
        uint32_t rx_pfc_p6_en        : 1;  /* Bit 6  : R/W RX Priority 6 PFC enable */
        uint32_t rx_pfc_p7_en        : 1;  /* Bit 7  : R/W RX Priority 7 PFC enable */

        uint32_t tx_pfc_p0_en        : 1;  /* Bit 8  : R/W TX Priority 0 PFC enable */
        uint32_t tx_pfc_p1_en        : 1;  /* Bit 9  : R/W TX Priority 1 PFC enable */
        uint32_t tx_pfc_p2_en        : 1;  /* Bit 10 : R/W TX Priority 2 PFC enable */
        uint32_t tx_pfc_p3_en        : 1;  /* Bit 11 : R/W TX Priority 3 PFC enable */
        uint32_t tx_pfc_p4_en        : 1;  /* Bit 12 : R/W TX Priority 4 PFC enable */
        uint32_t tx_pfc_p5_en        : 1;  /* Bit 13 : R/W TX Priority 5 PFC enable */
        uint32_t tx_pfc_p6_en        : 1;  /* Bit 14 : R/W TX Priority 6 PFC enable */
        uint32_t tx_pfc_p7_en        : 1;  /* Bit 15 : R/W TX Priority 7 PFC enable */

        uint32_t reserved16_19       : 4;  /* Bits 16–19 : RO Reserved */

        uint32_t tx_auto_xon         : 1;  /* Bit 20 : R/W TX Auto XON (send XON when pause request deasserts, PFC only) */

        uint32_t reserved21_24       : 4;  /* Bits 21–24 : RO Reserved */

        uint32_t rx_pfc_enable       : 1;  /* Bit 25 : R/W Priority Flow Control enable (RX) — do not enable with Flow Control RX */
        uint32_t tx_pfc_enable       : 1;  /* Bit 26 : R/W Priority Flow Control enable (TX) — do not enable with Flow Control TX */

        uint32_t reserved27_28       : 2;  /* Bits 27–28 : RO Reserved */

        uint32_t flow_ctrl_rx_enable : 1;  /* Bit 29 : R/W Standard Flow Control enable (RX pause handling) */
        uint32_t flow_ctrl_tx_enable : 1;  /* Bit 30 : R/W Standard Flow Control enable (TX pause generation) */

        uint32_t reserved31          : 1;  /* Bit 31 : RO Reserved */
    };
} xtemac_flow_ctl_cfg_t;

/* XTEMAC speed config word register */

typedef union xtemac_speed_cfg_word_u {

    uint32_t reg;

    struct {

        uint32_t reserved0_29     : 30; /* Bits 0–29  : RO  Reserved */
        uint32_t mac_speed        : 2;  /* Bits 30–31 : R/W MAC Speed Configuration
                                            00 = 10 Mbps
                                            01 = 100 Mbps
                                            10 = 1 Gbps
                                            Note: Hard-coded if core generated for only certain speeds */
    };
} xtemac_speed_cfg_t;

/* XTEMAC rx max frame config word register */

typedef union xtemac_rx_max_frame_cfg_word_u {

    uint32_t reg;

    struct {

        uint32_t rx_max_frame_len  : 15; /* Bits 0–14  : R/W RX Max Frame Length (0x0000–0x7FFF)
                                            Maximum frame length accepted by the receiver */
        uint32_t reserved15        : 1;  /* Bit 15     : RO  Reserved */
        uint32_t rx_max_frame_en   : 1;  /* Bit 16     : R/W RX Max Frame Enable
                                            0 = Standard max frame (1518/1522) depending on VLAN
                                            1 = Allow frames up to RX Max Frame Length
                                            Ignored if Jumbo Enable is set */
        uint32_t reserved17_31     : 15; /* Bits 17–31 : RO  Reserved */
    };
} xtemac_rx_max_frame_cfg_t;

/* XTEMAC tx max frame config word register */

typedef union xtemac_tx_max_frame_cfg_word_u {

    uint32_t reg;

    struct {

        uint32_t tx_max_frame_len  : 15; /* Bits 0–14  : R/W TX Max Frame Length (0x0000–0x7FFF)
                                            Maximum frame length allowed for transmission */
        uint32_t reserved15        : 1;  /* Bit 15     : RO  Reserved */
        uint32_t tx_max_frame_en   : 1;  /* Bit 16     : R/W TX Max Frame Enable
                                            0 = Standard max frame (1518/1522) depending on VLAN
                                            1 = Allow frames up to TX Max Frame Length
                                            Ignored if Jumbo Enable is set */
        uint32_t reserved17_31     : 15; /* Bits 17–31 : RO  Reserved */
    };
} xtemax_tx_max_frame_cfg_t;

/* XTEMAC per priority quanta/refresh register */

typedef union xtemac_per_priority_quanta_refresh_u {

    uint32_t reg;

    struct {

        uint32_t pause_quanta       : 16; /* Bits 0–15  : R/W Pause Quanta value
                                            Sets the quanta field to be transmitted in PFC frames
                                            for this priority (only present if PFC is enabled) */
        uint32_t pause_refresh      : 16; /* Bits 16–31 : R/W Pause Quanta refresh value
                                            Controls how frequently a new PFC frame refreshes the
                                            quanta for all active, enabled TX priorities */
    };
} xtemac_per_priority_quanta_refresh_t;

/* XTEMAC legacy pause refresh register */

typedef union xtemac_legacy_pause_refresh_u {

    uint32_t reg;

    struct {

        uint32_t reserved0_15       : 16; /* Bits 0–15  : RO  Reserved */
        uint32_t pause_refresh      : 16; /* Bits 16–31 : R/W Pause Quanta refresh value
                                            Controls the frequency of automatic pause refresh
                                            when PFC and XON/XOFF extended functionality are supported */
    };
} xtemac_legacy_pause_refresh_t;

/* XTEMAC id register */

typedef union xtemac_identifier_u {

    uint32_t reg;

    struct {

        uint32_t patch_level        : 8;  /* Bits 0–7   : RO  Patch Level
                                            0 = No patch
                                            1 = Rev1 */
        uint32_t reserved8_15       : 8;  /* Bits 8–15  : RO  Reserved */
        uint32_t minor_rev          : 8;  /* Bits 16–23 : RO  Minor Revision */
        uint32_t major_rev          : 8;  /* Bits 24–31 : RO  Major Revision */
    };
} xtemac_id_t;

/* XTEMAC ability register */

typedef union xtemac_ability_u {

    uint32_t reg;

    struct {

        uint32_t speed_10m_capable     : 1;  /* Bit 0  : RO  10 Mbps capability */
        uint32_t speed_100m_capable    : 1;  /* Bit 1  : RO  100 Mbps capability */
        uint32_t speed_1g_capable      : 1;  /* Bit 2  : RO  1 Gbps capability */
        uint32_t speed_2_5g_capable    : 1;  /* Bit 3  : RO  2.5 Gbps capability */
        uint32_t reserved4_7           : 4;  /* Bits 4–7   : RO  Reserved */
        uint32_t stats_counters_avail  : 1;  /* Bit 8  : RO  Statistics counters available */
        uint32_t half_duplex_capable   : 1;  /* Bit 9  : RO  Half duplex capable */
        uint32_t frame_filter_avail    : 1;  /* Bit 10 : RO  Frame filter available */
        uint32_t reserved11_15         : 5;  /* Bits 11–15 : RO  Reserved */
        uint32_t pfc_support           : 1;  /* Bit 16 : RO  Priority Flow Control supported */
        uint32_t reserved17_31         : 15; /* Bits 17–31 : RO  Reserved */
    };
} xtemac_ability_t;

/* XTEMAC uaw0 register */

typedef union xtemac_uaw0_u {

    uint32_t reg;

    struct {

        uint8_t byte0;   /* MAC[7:0]   */
        uint8_t byte1;   /* MAC[15:8]  */
        uint8_t byte2;   /* MAC[23:16] */
        uint8_t byte3;   /* MAC[31:24] */
    };
} xtemac_uaw0_t;

/* XTEMAC uaw1 register */

typedef union xtemac_uaw1_u {

    uint32_t reg;

    struct {

        uint8_t byte4;     /* MAC[39:32] */
        uint8_t byte5;     /* MAC[47:40] */
        uint8_t reserved0; /* Reserved   */
        uint8_t reserved1; /* Reserved   */
    };
} xtemac_uaw1_t;

/* XTEMAC */

typedef struct xtemac_s {

    volatile uint32_t rx_cfg_w0;                                       /* 0x400 */
    volatile union xtemac_rx_cfg_word1_u rx_cfg_w1;                    /* 0x404 */

    volatile union xtemac_tx_cfg_word_u tx_cfg;                        /* 0x408 */

    volatile union xtemac_flow_ctl_cfg_word_u flowCtl_cfg;             /* 0x40C */
    volatile union xtemac_speed_cfg_word_u speed_cfg;                  /* 0x410 */

    volatile union xtemac_rx_max_frame_cfg_word_u rx_maxFrame_cfg;     /* 0x414 */
    volatile union xtemac_tx_max_frame_cfg_word_u tx_maxFrame_cfg;     /* 0x418 */

    volatile uint32_t r0[25];                                          /* 0x41C - 0x47C */
    volatile union xtemac_per_priority_quanta_refresh_u pr_quanta[8];  /* 0x480 - 0x49C */

    volatile union xtemac_legacy_pause_refresh_u legacy_prfsh;         /* 0x4A0 */

    volatile uint32_t r1[21];                                          /* 0x4A4 - 0x4F4 */

    volatile union xtemac_identifier_u id;                             /* 0x4F8 */
    volatile union xtemac_ability_u ability;                           /* 0x4FC */
} xtemac_t;

/* XTEMAC MAC */

typedef union xtemac_mac_s {

    uint64_t reg;

    struct {

        volatile xtemac_uaw0_t uaw0; /* 0x00 */
        volatile xtemac_uaw1_t uaw1; /* 0x04 */
    };
} xtemac_mac_t;

static void xtemac_setmac(void const* mac);

/* XTEMAC type definitions */

void _xtemac_start(void) {

    _xtemac_phy();

    (XTEMAC -> rx_cfg_w1).reset = 1;
    while((XTEMAC -> rx_cfg_w1).reset) { }

    (XTEMAC -> tx_cfg).reset = 1;
    while((XTEMAC -> tx_cfg).reset) { }

    (XTEMAC -> tx_cfg).transmit_enable = false;
    (XTEMAC -> rx_cfg_w1).receiver_enable = false;

    uint8_t mac[] = XTEMAC_DEF_MACADDR;
    xtemac_setmac(mac);

    _ethdma_rxsgcyclic(XTEMAC_IO_BUFFER, XTEMAC_MAX_FRAME);
}

void _xtemac_mac(void *const mac, const size_t len) {

    if (len > XTEMAC_MAX_MAC) {
        return;
    }

    xtemac_mac_t xmac;
    xmac.reg = (XTEMAC_MAC -> reg);

    uint8_t *maddr = ((uint8_t *) mac);

    /* msb of a MAC register is a first byte of a MAC sequence  */

    maddr[0] = xmac.uaw1.byte5;
    maddr[1] = xmac.uaw1.byte4;

    maddr[2] = xmac.uaw0.byte3;
    maddr[3] = xmac.uaw0.byte2;
    maddr[4] = xmac.uaw0.byte1;
    maddr[5] = xmac.uaw0.byte0;
}

void _xtemac_trxdisable(void) {

    (XTEMAC -> tx_cfg).transmit_enable = false;
    (XTEMAC -> rx_cfg_w1).receiver_enable = false;
}

void _xtemac_trxenable(void) {

    phylink_t phy;
    if (!_xtemac_phylink(&phy)) { /* No link */
        return;
    }

    xtemac_rx_cfg_w1_t rx = { 0 };

    rx.vlan_enable = true;
    rx.receiver_enable = true;
    rx.ctrl_len_chk_dis = true;
    rx.len_type_chk_dis = true;
    rx.inband_fcs_enable = false;
    rx.jumbo_frame_enable = false;

    rx.half_duplex = (phy.link == LINK_HALF_DUPLEX);

    xtemac_rx_max_frame_cfg_t rx_cfg = { 0 };

    rx_cfg.rx_max_frame_en = true;
    rx_cfg.rx_max_frame_len = XTEMAC_MAX_FRAME;
    
    xtemac_speed_cfg_t mac_cfg = { 0 };

    mac_cfg.mac_speed = 
        (phy.speed == SPEED_10MBPS) ? XTEMAC_10MBPS_SPEED : XTEMAC_100MPBS_SPEED;

    (XTEMAC -> speed_cfg).reg = mac_cfg.reg;
    (XTEMAC -> rx_maxFrame_cfg).reg = rx_cfg.reg;

    (XTEMAC -> rx_cfg_w1).reg = rx.reg;
}

/****************************************************************************/

static void xtemac_setmac(void const* mac) {

    /* first MAC seq byte is msb of a MAC register */

    uint8_t *setmac = (uint8_t *) mac;
    xtemac_mac_t xmac = { .uaw0.byte0 = setmac[5],
                          .uaw0.byte1 = setmac[4],
                          .uaw0.byte2 = setmac[3],
                          .uaw0.byte3 = setmac[2],
                        
                          .uaw1.byte4 = setmac[1],
                          .uaw1.byte5 = setmac[0] };

    (XTEMAC_MAC -> reg) = xmac.reg;
}