#include <stddef.h>
#include <stdlib.h>

#include <stdint.h>
#include <stdbool.h>

#include "kernel_signal.h"

#include "sys/xintc.h"
#include "sys/xtemac_phy.h"

#define XTEMAC_PHY_IRQ                          (11)

#define XTEMAC_BASE                             (0x44A60000)

#define XTEMAC_MDIO_BASE                        (XTEMAC_BASE + 0x500)
#define XTEMAC_MDIO_IC_BASE                     (XTEMAC_BASE + 0x600)

#define XTEMAC_MDIO_PHY                         ((xtemac_phy_mdio_t *) XTEMAC_MDIO_BASE)
#define XTEMAC_MDIO_IC                          ((xtemac_mdio_intc_t *) XTEMAC_MDIO_IC_BASE)

#define XTEMAC_MDIO_WRITE                       (0x01) /* read register operation */
#define XTEMAC_MDIO_READ                        (0x02) /* write register operation */

#define XTEMAC_MDIO_PHY_ADDR                    (0x01) /* PHY MDIO addr */
#define XTEMAC_MDIO_CLOCK_DIV                   (0x14) /* PHY MDIO clk divide calib */

#define XTEMAC_MDIO_PHY_BMCR                    (0) /* basic mode control register */
#define XTEMAC_MDIO_PHY_BMSR                    (1) /* basic mode status register */
#define XTEMAC_MDIO_PHY_ID1                     (2) /* phy id 1 register */
#define XTEMAC_MDIO_PHY_ID2                     (3) /* phy id 2 register */

#define XTEMAC_MDIO_LAN8720A_MCRSR              (17) /* lan8720a mode control/status register */
#define XTEMAC_MDIO_LAN8720A_SM                 (18) /* lan8720a special modes register */
#define XTEMAC_MDIO_LAN8720A_ISR                (29) /* lan8720a irq status register */
#define XTEMAC_MDIO_LAN8720A_IMR                (30) /* lan8720a irq mask register */
#define XTEMAC_MDIO_LAN8720A_SCSR               (31) /* lan8720a special control/status register */

#define XTEMAC_PHY_HALF_DUPLEX                  (0)  /* PHY half duplex mode  */
#define XTEMAC_PHY_FULL_DUPLEX                  (1)  /* PHY full duplex link mode */

/********************************* PHY data def *****************************************/

typedef enum
{
    LAN8720_10M_HALF  = 0b001,
    LAN8720_10M_FULL  = 0b101,
    LAN8720_100M_HALF = 0b010,
    LAN8720_100M_FULL = 0b110
} lan8720_speed_t;

/********************************** PHY registers ***************************************/

// IEEE 802.3 index 0 register

typedef struct phy_bmcr_s {

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
    uint16_t autoneg_enable     : 1;  /* Bit 12    : Auto-Negotiation Enable */
    uint16_t speed_select       : 1;  /* Bit 13    : Speed Select
                                            0 = 10 Mbps
                                            1 = 100 Mbps
                                            Ignored if Auto-Neg enabled */
    uint16_t loopback           : 1;  /* Bit 14    : Loopback */
    uint16_t reset              : 1;  /* Bit 15    : Software Reset
                                            Self-clearing
                                            Do NOT set other bits when writing 1 */
} phy_bmcr_t;

// IEEE 802.3 index 1 register

typedef struct phy_bmsr_s {

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

} phy_bmsr_t;

// IEEE 802.3 index 3 register

typedef struct phyid2_s {

    uint16_t revision        : 4;  /* Bits 3:0  : Revision number */
    uint16_t model           : 6;  /* Bits 9:4  : Model number */
    uint16_t oui_bits        : 6;  /* Bits 15:10: OUI bits 19–24 */

} phyid2_t;

// lan8720a index 17 register

typedef struct lan8720a_mcrsr_s {

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

} lan8720a_mcrsr_t;

// lan8720a index 18 register

typedef struct lan8720a_sm_s {

    uint16_t phyad          : 5;  /* Bits 4:0  : PHY address */
    uint16_t mode           : 3;  /* Bits 7:5  : Operating mode */
    uint16_t reserved8_13   : 6;  /* Bits 13:8 : Reserved (RO) */
    uint16_t reserved14     : 1;  /* Bit 14    : Must write 1 */
    uint16_t reserved15     : 1;  /* Bit 15    : Reserved (RO) */

} lan8720a_sm_t;

// lan8720a index 29 register

typedef struct lan8720a_isr_s {

    uint16_t reserved0        : 1;  /* Bit 0  : Reserved */
    uint16_t autoneg_page_rx  : 1;  /* Bit 1  : Auto-Neg Page Received (LH) */
    uint16_t par_detect_fault : 1;  /* Bit 2  : Parallel Detection Fault (LH) */
    uint16_t autoneg_lp_ack   : 1;  /* Bit 3  : Auto-Neg LP Acknowledge (LH) */
    uint16_t linkdown         : 1;  /* Bit 4  : Link Down (LH) */
    uint16_t remote_fault     : 1;  /* Bit 5  : Remote Fault (LH) */
    uint16_t autoneg_cmplt    : 1;  /* Bit 6  : Auto-Neg Complete (LH) */
    uint16_t energyon         : 1;  /* Bit 7  : ENERGYON event (LH) */
    uint16_t reserved8_15     : 8;  /* Bits 8–15 : Reserved */

} lan8720a_isr_t;

// lan8720a index 30 register

typedef struct lan8720a_imr_s {

    uint16_t reserved0               : 1;  /* Bit 0  : Reserved */

    uint16_t autoneg_page_rx_en      : 1;  /* Bit 1  : Enable Page Received interrupt */
    uint16_t par_detect_fault_irq_en : 1;  /* Bit 2  : Enable Parallel Detect Fault */
    uint16_t autoneg_lpack_irq_en    : 1;  /* Bit 3  : Enable LP Acknowledge */
    uint16_t linkdown_irq_en         : 1;  /* Bit 4  : Enable Link Down */
    uint16_t remote_fault_irq_en     : 1;  /* Bit 5  : Enable Remote Fault */
    uint16_t autoneg_irq_en          : 1;  /* Bit 6  : Enable Auto-Neg Complete */
    uint16_t energyon_irq_en         : 1;  /* Bit 7  : Enable ENERGYON */

    uint16_t reserved8_15            : 8;  /* Bits 8–15 : Reserved */

} lan8720a_imr_t;

// lan8720a index 31 register

typedef struct lan8720a_scsr_s {

    uint16_t reserved0_1   : 2;  /* Bits 0–1   : Reserved */
    uint16_t hcd_speed     : 3;  /* Bits 2–4   : Speed Indication
                                     001 = 10M Half
                                     101 = 10M Full
                                     010 = 100M Half
                                     110 = 100M Full */
    uint16_t reserved5_14  : 10; /* Bits 5–14  : Reserved */
    uint16_t autoneg_done  : 1;  /* Bit 15     : Auto-Negotiation Done */

} lan8720a_scsr_t;

typedef union phy_reg_u {

    volatile uint16_t reg;
    
    volatile struct phy_bmcr_s bmcr;         /* Index 0 */
    volatile struct phy_bmsr_s bmsr;         /* Index 1 */
    volatile struct phyid2_s id2;            /* Index 2 (id_1) Index 3 (id_2) */
    volatile struct lan8720a_mcrsr_s mcrscr; /* Index 17 */
    volatile struct lan8720a_sm_s smr;       /* Index 18 */
    volatile struct lan8720a_isr_s isr;      /* Index 29 */
    volatile struct lan8720a_imr_s imr;      /* Index 30 */
    volatile struct lan8720a_scsr_s scsr;    /* Index 31 */

} phy_reg_t;

/* XTEMAC AXI to mdio PHY translation */

/* MDIO setup word register */

typedef union xtemac_mdio_setup_word_u {

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

} xtemac_mdio_setup_t;

/* MDIO control word register */

typedef union xtemac_mdio_control_word_u {

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

} xtemac_mdio_control_t;

/* MDIO write data register */

typedef union xtemac_mdio_write_data_u {

    uint32_t reg;

    struct {

        uint32_t wdata           : 16; /* Bits 0–15  : R/W MDIO Write Data
                                           Data to be written to the selected PHY register */
        uint32_t reserved16_31   : 16; /* Bits 16–31 : RO  Reserved */
    };

} xtemac_mdio_wdata_t;

/* MDIO read data register */

typedef union xtemac_mdio_read_data_u {

    uint32_t reg;

    struct {

        uint32_t rdata           : 16; /* Bits 0–15  : RO  MDIO Read Data
                                           Valid only when MDIO Ready (bit 7 of MDIO Control Word) is High */
        uint32_t mdio_ready      : 1;  /* Bit 16     : RO  Copy of MDIO Control Word bit 7
                                           Indicates MDIO interface is ready for a new transfer */
        uint32_t reserved17_31   : 15; /* Bits 17–31 : RO  Reserved */
    };

} xtemac_mdio_rdata_t;

typedef union xtemac_mdio_ic_u {

    uint32_t reg;

    struct {

        uint32_t mdioc_irq   : 1;  /* Bit 0 : MDIO I/O completed irq */

        uint32_t tx_ptp_irq  : 1;  /* Bit 1 : tx ptp irq */
        uint32_t rx_ptp_irq  : 1;  /* Bit 2 : rx ptp irq */
        uint32_t tmr_ptp_irq : 1;  /* Bit 3 : timer ptp irq */

        uint32_t reserved    : 28; /* Bit 4-31: reserved*/
    };
} xtemac_mdio_irq_t;

/* XTEMAC phy mdio */

typedef struct xtemac_phy_mdio_s {

    volatile union xtemac_mdio_setup_word_u stp;   /* 0x000 */
    volatile union xtemac_mdio_control_word_u cr;  /* 0x004 */
    volatile union xtemac_mdio_write_data_u wdr;   /* 0x008 */
    volatile union xtemac_mdio_read_data_u rdr;    /* 0x00C */

} xtemac_phy_mdio_t;

typedef struct xtemc_mdio_ic_s {

    volatile union xtemac_mdio_ic_u isr;           /* 0x000 */

    volatile uint32_t r0[3];                       /* 0x004 - 0x00C */
    volatile union xtemac_mdio_ic_u ipr;           /* 0x010 */

    volatile uint32_t r1[3];                       /* 0x014 - 0x01C */
    volatile union xtemac_mdio_ic_u ier;           /* 0x020 */

    volatile uint32_t r2[3];                       /* 0x024 - 0x02C */
    volatile union xtemac_mdio_ic_u icr;           /* 0x030 */

} xtemac_mdio_intc_t;

static volatile bool link_sig = false;

static __attribute__((fast_interrupt)) void on_phyIRQ(void);

static bool read_phyReg(uint8_t addr, uint16_t *regv);
static bool write_phyReg(uint8_t addr, const uint16_t regv);

uint16_t _xtemac_phyreg(uint8_t addr) {
    uint16_t regv;
    read_phyReg(addr, &regv);
    return regv;
}

void _xtemac_phy(void) {

    link_sig = false;

    volatile xtemac_mdio_setup_t mdio_setup = { 0 };

    mdio_setup.mdio_enable = true;
    mdio_setup.clock_divide = XTEMAC_MDIO_CLOCK_DIV;

    (XTEMAC_MDIO_PHY -> stp).reg = mdio_setup.reg;

    _xtemac_phyReset(); // reset PHY

    volatile phy_reg_t phy = { 0 };
    
    phy.bmcr.autoneg_enable = true;

    write_phyReg(XTEMAC_MDIO_PHY_BMCR, phy.reg); // enable autoneg

    _xintc_disableIRQ(XTEMAC_PHY_IRQ);    
    _xintc_enableIRQ(XTEMAC_PHY_IRQ, on_phyIRQ);

    phy.reg = 0;

    phy.imr.autoneg_irq_en = true;
    phy.imr.linkdown_irq_en = true;

    write_phyReg(XTEMAC_MDIO_LAN8720A_IMR, phy.reg); /* unmask PHY interrupt sources */
}

void _xtemac_phyReset(void) {

    volatile phy_reg_t phy = { 0 };

    phy.bmcr.reset = true;
    write_phyReg(XTEMAC_MDIO_PHY_BMCR, phy.reg);

    do {
        read_phyReg(XTEMAC_MDIO_PHY_BMCR, (uint16_t *) &phy);
    } while(phy.bmcr.reset); // wait for reset done
}

bool _xtemac_phylinkSgl(void) {
    return link_sig;
}

bool _xtemac_phylink(phylink_t *lk) {

    if (lk == NULL) {
        return false;
    }

    volatile phy_reg_t phy = { 0 };
    read_phyReg(XTEMAC_MDIO_PHY_BMSR, (uint16_t *) &phy);

    bool linkstatus = 
        phy.bmsr.link_satus && phy.bmsr.autoneg_cmplt;

    read_phyReg(XTEMAC_MDIO_LAN8720A_SCSR, (uint16_t *) &phy);

    if (!linkstatus) {
        return linkstatus;
    }

    switch (phy.scsr.hcd_speed) {

        case LAN8720_10M_FULL:
        case LAN8720_100M_FULL: {
            
            lk -> link = LINK_FULL_DUPLEX;
            lk -> speed = (phy.scsr.hcd_speed == LAN8720_10M_FULL) ? SPEED_10MBPS : SPEED_100MBPS;
        } break;

        case LAN8720_10M_HALF:
        case LAN8720_100M_HALF: {

            lk -> link = LINK_HALF_DUPLEX;
            lk -> speed = (phy.scsr.hcd_speed == LAN8720_10M_HALF) ? SPEED_10MBPS : SPEED_100MBPS;
        } break;
    
        default:
            break;
    }

    link_sig = false;
    return linkstatus;
}

uint32_t _xtemac_phyid(void) {

    if (!(XTEMAC_MDIO_PHY -> rdr).mdio_ready) {
        return 0;
    }

    uint16_t phy_id1, phy_id2;

    bool id1_ok = read_phyReg(XTEMAC_MDIO_PHY_ID1, &phy_id1);
    bool id2_ok = read_phyReg(XTEMAC_MDIO_PHY_ID2, &phy_id2);

    return (id1_ok && id2_ok) ? ((phy_id1 << 16) | phy_id2) : 0;
}

static bool read_phyReg(uint8_t addr, uint16_t *regv) {

    if (!(XTEMAC_MDIO_PHY -> rdr).mdio_ready) {
        return false;
    }

    (XTEMAC_MDIO_IC -> icr).mdioc_irq = true;     // clear MDIO ioc flag
    while((XTEMAC_MDIO_IC -> isr).mdioc_irq) { }; // wait MDIO ioc cleared

    xtemac_mdio_control_t mdio_cr = { 0 };

    mdio_cr.initiate = true;
    mdio_cr.op = XTEMAC_MDIO_READ;

    mdio_cr.phy_reg_addr = addr;
    mdio_cr.phy_addr = XTEMAC_MDIO_PHY_ADDR;

    (XTEMAC_MDIO_PHY -> cr).reg = mdio_cr.reg; // initiate read phy register

    /* wait MDIO ioc flag */
    while(!(XTEMAC_MDIO_IC -> isr).mdioc_irq) { }

    *regv = (XTEMAC_MDIO_PHY -> rdr).rdata;
    return true;
}

static bool write_phyReg(uint8_t addr, const uint16_t regv) {

    if (!(XTEMAC_MDIO_PHY -> cr).mdio_ready) {
        return false;
    }

    (XTEMAC_MDIO_IC -> icr).mdioc_irq = true;     // clear MDIO ioc flag
    while((XTEMAC_MDIO_IC -> isr).mdioc_irq) { }; // wait MDIO ioc cleared

    xtemac_mdio_control_t mdio_cr = { 0 };

    mdio_cr.initiate = true;
    mdio_cr.op = XTEMAC_MDIO_WRITE;

    mdio_cr.phy_reg_addr = addr;
    mdio_cr.phy_addr = XTEMAC_MDIO_PHY_ADDR;

    (XTEMAC_MDIO_PHY -> wdr).wdata = regv;
    (XTEMAC_MDIO_PHY -> cr).reg = mdio_cr.reg; // initiate write phy register

    /* wait MDIO ioc flag */
    while(!(XTEMAC_MDIO_IC -> isr).mdioc_irq) { }

    return true;
}

static void on_phyIRQ(void) {

    volatile phy_reg_t phy;
    read_phyReg(XTEMAC_MDIO_LAN8720A_ISR, (uint16_t *) &phy);

    link_sig = phy.isr.linkdown || phy.isr.autoneg_cmplt;
    _kernel_raise(SIGINT);
}