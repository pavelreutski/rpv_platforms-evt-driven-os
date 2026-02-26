#include <stdint.h>

#include "sys/xtemac.h"
#include "sys/xtemac_phy.h"

#define XTEMAC_BASE                     (0x44A60000)

#define XTEMAC_REG_BASE                 (XTEMAC_BASE + 0x400)

#define XTEMAC                          ((xtemac_t *) XTEMAC_REG_BASE)

enum xtemac_speed_u : uint8_t {

    XTEMAC_10MBPS_SPEED  = 0x00,
    XTEMAC_100MPBS_SPEED = 0x01,
    XTEMAC_1GBPS_SPEED   = 0x02
};

/********************************* XTEMAC registers *************************************/

/* XTEMAC rx config word 1 register */

union xtemac_rx_cfg_word1_u {

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
};

/* XTEMAC tx config word register */

union xtemac_tx_cfg_word_u {
    
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
};

/*XTEMAC flow control config word register */

union xtemac_flow_ctl_cfg_word_u {

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
};

/* XTEMAC speed config word register */

union xtemac_speed_cfg_word_u {

    uint32_t reg;

    struct {

        uint32_t reserved0_29     : 30; /* Bits 0–29  : RO  Reserved */
        uint32_t mac_speed        : 2;  /* Bits 30–31 : R/W MAC Speed Configuration
                                            00 = 10 Mbps
                                            01 = 100 Mbps
                                            10 = 1 Gbps
                                            Note: Hard-coded if core generated for only certain speeds */
    };
};

/* XTEMAC rx max frame config word register */

union xtemac_rx_max_frame_cfg_word_u {

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
};

/* XTEMAC tx max frame config word register */

union xtemac_tx_max_frame_cfg_word_u {

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
};

/* XTEMAC per priority quanta/refresh register */

union xtemac_per_priority_quanta_refresh_u {

    uint32_t reg;

    struct {

        uint32_t pause_quanta       : 16; /* Bits 0–15  : R/W Pause Quanta value
                                            Sets the quanta field to be transmitted in PFC frames
                                            for this priority (only present if PFC is enabled) */
        uint32_t pause_refresh      : 16; /* Bits 16–31 : R/W Pause Quanta refresh value
                                            Controls how frequently a new PFC frame refreshes the
                                            quanta for all active, enabled TX priorities */
    };
};

/* XTEMAC legacy pause refresh register */

union xtemac_legacy_pause_refresh_u {

    uint32_t reg;

    struct {

        uint32_t reserved0_15       : 16; /* Bits 0–15  : RO  Reserved */
        uint32_t pause_refresh      : 16; /* Bits 16–31 : R/W Pause Quanta refresh value
                                            Controls the frequency of automatic pause refresh
                                            when PFC and XON/XOFF extended functionality are supported */
    };
};

/* XTEMAC id register */

union xtemac_identifier_u {

    uint32_t reg;

    struct {

        uint32_t patch_level        : 8;  /* Bits 0–7   : RO  Patch Level
                                            0 = No patch
                                            1 = Rev1 */
        uint32_t reserved8_15       : 8;  /* Bits 8–15  : RO  Reserved */
        uint32_t minor_rev          : 8;  /* Bits 16–23 : RO  Minor Revision */
        uint32_t major_rev          : 8;  /* Bits 24–31 : RO  Major Revision */
    };
};

/* XTEMAC ability register */

union xtemac_ability_u {

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
};

union xtemac_ic_u {

    uint32_t reg;

    struct {

        uint32_t mdioc_irq   : 1;  /* Bit 0 : MDIO I/O completed irq */

        uint32_t tx_ptp_irq  : 1;  /* Bit 1 : tx ptp irq */
        uint32_t rx_ptp_irq  : 1;  /* Bit 2 : rx ptp irq */
        uint32_t tmr_ptp_irq : 1;  /* Bit 3 : timer ptp irq */

        uint32_t reserved    : 28; /* Bit 4-31: reserved*/
    };
};

/* XTEMAC */

struct xtemac_s {

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

    /* MDIO */

    volatile uint32_t phy[4];                                          /* 0x500 - 0x50C */

    /************************************************/

    volatile uint32_t r2[60];                                          /* 0x510 - 0x5FC */
    volatile union xtemac_ic_u isr;                                    /* 0x600 */

    volatile uint32_t r3[3];                                           /* 0x604 - 0x60C */
    volatile union xtemac_ic_u ipr;                                    /* 0x610 */

    volatile uint32_t r4[3];                                           /* 0x614 - 0x61C */
    volatile union xtemac_ic_u ier;                                    /* 0x620 */

    volatile uint32_t r5[3];                                           /* 0x624 - 0x62C */
    volatile union xtemac_ic_u icr;                                    /* 0x630 */
};

/* XTEMAC type definitions */

typedef struct xtemac_s xtemac_t;

typedef union xtemac_ic_u xtemac_intc_t;

typedef union xtemac_rx_cfg_word1_u xtemac_rx_cfg_w1_t;

typedef union xtemac_tx_cfg_word_u xtemax_tx_cfg_t;
typedef union xtemac_flow_ctl_cfg_word_u xtemac_flow_ctl_cfg_t;

typedef union xtemac_speed_cfg_word_u xtemax_speed_cfg_t;

typedef union xtemac_rx_max_frame_cfg_word_u xtemac_rx_max_frame_cfg_t;
typedef union xtemac_tx_max_frame_cfg_word_u xtemax_tx_max_frame_cfg_t;

typedef union xtemac_per_priority_quanta_refresh_u xtemac_per_priority_quanta_refresh_t;
typedef union xtemac_legacy_pause_refresh_u xtemac_legacy_pause_refresh_t;

typedef union xtemac_identifier_u xtemac_id_t;
typedef union xtemac_ability_u xtemac_ability_t;

void xtemac_start(void) {
    xtemac_phy();
}

void xtemac_id(char *s, const uint8_t len) {

    (void) s;
    (void) len;
}