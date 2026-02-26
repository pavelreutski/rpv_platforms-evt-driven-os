#include <stdint.h>
#include <stdbool.h>

#include "sys/xtemac_phy.h"

#define XTEMAC_BASE                     (0x44A60000)

#define XTEMAC_PHY_BASE                 (XTEMAC_BASE + 0x500)

#define XTEMAC_PHY                      ((xtemac_phy_mdio_t *) XTEMAC_PHY_BASE)

/********************************** PHY registers ***************************************/

// IEEE 802.3 index 0 register

struct phy_bmcr_s {

    uint16_t reserved0_7        : 8;  /* Bits 0–7  : Reserved (write 0) */
    uint16_t duplex_mode        : 1;  /* Bit 8     : Duplex Mode
                                            0 = Half
                                            1 = Full
                                            Ignored if Auto-Neg enabled */
    uint16_t restart_auto_neg   : 1;  /* Bit 9     : Restart Auto-Negotiation
                                            Self-clearing */
    uint16_t isolate            : 1;  /* Bit 10    : Isolate RMII */
    uint16_t power_down         : 1;  /* Bit 11    : Power Down
                                            Auto-Neg must be 0 before setting */
    uint16_t auto_neg_enable    : 1;  /* Bit 12    : Auto-Negotiation Enable */
    uint16_t speed_select       : 1;  /* Bit 13    : Speed Select
                                            0 = 10 Mbps
                                            1 = 100 Mbps
                                            Ignored if Auto-Neg enabled */
    uint16_t loopback           : 1;  /* Bit 14    : Loopback */
    uint16_t reset              : 1;  /* Bit 15    : Software Reset
                                            Self-clearing
                                            Do NOT set other bits when writing 1 */
};

// IEEE 802.3 index 1 register

struct phy_bmsr_s {

    uint16_t ext_avail               : 1;  /* Bit 0  : Extended capability regs supported */
    uint16_t jabber_detect           : 1;  /* Bit 1  : Jabber detected (RO/LH) */
    uint16_t link_satus              : 1;  /* Bit 2  : Link status
                                                1 = link up
                                                0 = link down
                                                (RO/LL – latch low) */
    uint16_t autoneg_avail           : 1;  /* Bit 3  : Auto-negotiation capable */
    uint16_t remote_fault            : 1;  /* Bit 4  : Remote fault detected (RO/LH) */
    uint16_t autoneg_cmplt           : 1;  /* Bit 5  : Auto-negotiation complete */

    uint16_t reserved6_7             : 2;  /* Bits 6–7 : Reserved */

    uint16_t ext_status              : 1;  /* Bit 8  : Extended status reg (Reg 15) available */

    uint16_t t2_half_duplex          : 1;  /* Bit 9  : 100BASE-T2 Half Duplex ability */
    uint16_t t2_full_duplex          : 1;  /* Bit 10 : 100BASE-T2 Full Duplex ability */

    uint16_t t10_half_duplex         : 1;  /* Bit 11 : 10BASE-T Half Duplex ability */
    uint16_t t10_full_duplex         : 1;  /* Bit 12 : 10BASE-T Full Duplex ability */

    uint16_t tx_half_duplex          : 1;  /* Bit 13 : 100BASE-TX Half Duplex ability */
    uint16_t tx_full_duplex          : 1;  /* Bit 14 : 100BASE-TX Full Duplex ability */

    uint16_t t4_ability              : 1;  /* Bit 15 : 100BASE-T4 ability */    
};

// IEEE 802.3 index 3 register

struct phyid2_s {

    uint16_t revision        : 4;  /* Bits 3:0  : Revision number */
    uint16_t model           : 6;  /* Bits 9:4  : Model number */
    uint16_t oui_bits        : 6;  /* Bits 15:10: OUI bits 19–24 */
};

// lan8720a index 17 register

struct lan8720a_mcrsr_s {

    uint16_t reserved0       : 1;  /* Bit 0  : Reserved (RO) */
    uint16_t energy_on       : 1;  /* Bit 1  : Energy detected (RO)
                                        Clears after ~256ms without energy */
    uint16_t reserved2_5     : 4;  /* Bits 2–5 : Reserved (RO) */
    uint16_t altint          : 1;  /* Bit 6  : Alternate Interrupt mode */
    uint16_t reserved7_8     : 2;  /* Bits 7–8 : Reserved (RO) */
    uint16_t far_loopback    : 1;  /* Bit 9  : Far Loopback (100BASE-TX) */
    uint16_t reserved10_12   : 3;  /* Bits 10–12 : Reserved (RO) */
    uint16_t edpwrdown       : 1;  /* Bit 13 : Energy Detect Power-Down */
    uint16_t reserved14_15   : 2;  /* Bits 14–15 : Reserved (RO) */
};

// lan8720a index 18 register

struct lan8720a_sm_s {

    uint16_t phyad          : 5;  /* Bits 4:0  : PHY address */
    uint16_t mode           : 3;  /* Bits 7:5  : Operating mode */
    uint16_t reserved8_13   : 6;  /* Bits 13:8 : Reserved (RO) */
    uint16_t reserved14     : 1;  /* Bit 14    : Must write 1 */
    uint16_t reserved15     : 1;  /* Bit 15    : Reserved (RO) */
};

// lan8720a index 29 register

struct lan8720a_isr_s {

    uint16_t reserved0        : 1;  /* Bit 0  : Reserved */
    uint16_t autoneg_page_rx  : 1;  /* Bit 1  : Auto-Neg Page Received (LH) */
    uint16_t par_detect_fault : 1;  /* Bit 2  : Parallel Detection Fault (LH) */
    uint16_t autoneg_lp_ack   : 1;  /* Bit 3  : Auto-Neg LP Acknowledge (LH) */
    uint16_t link_down        : 1;  /* Bit 4  : Link Down (LH) */
    uint16_t remote_fault     : 1;  /* Bit 5  : Remote Fault (LH) */
    uint16_t autoneg_cmplt    : 1;  /* Bit 6  : Auto-Neg Complete (LH) */
    uint16_t energyon         : 1;  /* Bit 7  : ENERGYON event (LH) */
    uint16_t reserved8_15     : 8;  /* Bits 8–15 : Reserved */
};

// lan8720a index 30 register

struct lan8720a_imr_s {

    uint16_t reserved0               : 1;  /* Bit 0  : Reserved */

    uint16_t autoneg_page_rx_en      : 1;  /* Bit 1  : Enable Page Received interrupt */
    uint16_t par_detect_fault_irq_en : 1;  /* Bit 2  : Enable Parallel Detect Fault */
    uint16_t autoneg_lpack_irq_en    : 1;  /* Bit 3  : Enable LP Acknowledge */
    uint16_t link_down_irq_en        : 1;  /* Bit 4  : Enable Link Down */
    uint16_t remote_fault_irq_en     : 1;  /* Bit 5  : Enable Remote Fault */
    uint16_t autoneg_irq_en          : 1;  /* Bit 6  : Enable Auto-Neg Complete */
    uint16_t energyon_irq_en         : 1;  /* Bit 7  : Enable ENERGYON */

    uint16_t reserved8_15            : 8;  /* Bits 8–15 : Reserved */    
};

union phy_reg_u {

    volatile uint16_t reg;
    
    volatile struct phy_bmcr_s bmcr;         /* Index 0 */
    volatile struct phy_bmsr_s bmsr;         /* Index 1 */
    volatile struct phyid2_s id1;            /* Index 3 */
    volatile struct lan8720a_mcrsr_s mcrscr; /* Index 17 */
    volatile struct lan8720a_sm_s smr;       /* Index 18 */
    volatile struct lan8720a_isr_s isr;      /* Index 29 */
    volatile struct lan8720a_imr_s imr;      /* Index 30 */
};

/* XTEMAC AXI to mdio PHY translation */

/* MDIO setup word register */

union xtemac_mdio_setup_word_u {

    uint32_t reg;

    struct {

        uint32_t clock_divide       : 6;  /* Bits 0–5  : R/W MDIO Clock Divide
                                               Sets the clock divider for MDIO transactions
                                               Must be non-zero for MDIO Enable to take effect. 
                                                 f_mdc = f_axi_aclk / (1 + clk_div * 2) */
        uint32_t mdio_enable        : 1;  /* Bit 6     : R/W MDIO Enable
                                               0 = MDIO interface disabled
                                               1 = MDIO interface enabled (requires non-zero clock divide) */
        uint32_t reserved7_31       : 25; /* Bits 7–31 : RO  Reserved */
    };
};

/* MDIO control word register */

union xtemac_mdio_control_word_u {

    uint32_t reg;

    struct {

        uint32_t reserved0_6   : 7;  /* Bits 0–6   : RO  Reserved */
        uint32_t mdio_ready    : 1;  /* Bit 7      : RO  MDIO Ready
                                         1 = MDIO enabled and ready for new transfer */
        uint32_t reserved8_10  : 3;  /* Bits 8–10  : RO  Reserved */
        uint32_t initiate      : 1;  /* Bit 11     : WO  Initiate MDIO transfer
                                         Write 1 to start read/write transaction */
        uint32_t reserved12_13 : 2;  /* Bits 12–13 : RO  Reserved */
        uint32_t op            : 2;  /* Bits 14–15 : WO  MDIO Operation
                                         01 = Write, 10 = Read */
        uint32_t phy_reg_addr  : 5;  /* Bits 16–20 : R/W MDIO Register Address */
        uint32_t reserved21_23 : 3;  /* Bits 21–23 : RO  Reserved */
        uint32_t phy_addr      : 5;  /* Bits 24–28 : R/W MDIO PHY Address */
        uint32_t reserved29_31 : 3;  /* Bits 29–31 : RO  Reserved */
    };
};

/* MDIO write data register */

union xtemac_mdio_write_data_u {

    uint32_t reg;

    struct {

        uint32_t wdata           : 16; /* Bits 0–15  : R/W MDIO Write Data
                                           Data to be written to the selected PHY register */
        uint32_t reserved16_31   : 16; /* Bits 16–31 : RO  Reserved */
    };
};

/* MDIO read data register */

union xtemac_mdio_read_data_u {

    uint32_t reg;

    struct {

        uint32_t rdata           : 16; /* Bits 0–15  : RO  MDIO Read Data
                                           Valid only when MDIO Ready (bit 7 of MDIO Control Word) is High */
        uint32_t mdio_ready      : 1;  /* Bit 16     : RO  Copy of MDIO Control Word bit 7
                                           Indicates MDIO interface is ready for a new transfer */
        uint32_t reserved17_31   : 15; /* Bits 17–31 : RO  Reserved */
    };
};

/* XTEMAC phy mdio */

struct xtemac_phy_mdio_s {

    union xtemac_mdio_setup_word_u stp;   /* 0x000 */
    union xtemac_mdio_control_word_u cr;  /* 0x004 */
    union xtemac_mdio_write_data_u wdata; /* 0x008 */
    union xtemac_mdio_read_data_u rdata;  /* 0x00C */
};

/* XTEMAC MDIO type definitions */

typedef struct xtemac_phy_mdio_s xtemac_phy_mdio_t;

typedef union xtemac_mdio_setup_word_u xtemac_mdio_setup_t;
typedef union xtemac_mdio_control_word_u xtemac_mdio_control_t;

typedef union xtemac_mdio_write_data_u xtemac_mdio_wdata_t;
typedef union xtemac_mdio_read_data_u xtemac_mdio_rdata_t;

/* PHY type definitions */

/* IEEE 802.3 registers */

typedef union phyid2_u phyid2_t;
typedef union phy_bmcr_u phy_bmcr_t;
typedef union phy_bmsr_u phy_bmsr_t;

/* LAN8720A specific registers */

typedef union lan8720a_sm_u lan8720a_sm_t;
typedef union lan8720a_isr_u lan8720a_isr_t;
typedef union lan8720a_imr_u lan8720a_imr_t;
typedef union lan8720a_mcrsr_u lan8720a_mcrsr_t;

typedef union phy_reg_u phy_reg_t;

void xtemac_phy(void) {

}

bool xtemac_phylink(void) {
    return false;
}

void xtemac_phyid(char *s, const uint8_t len) {

    (void) s;
    (void) len;
}