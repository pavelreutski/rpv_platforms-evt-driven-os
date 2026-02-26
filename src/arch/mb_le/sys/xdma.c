#include <stdint.h>
#include <string.h>

#include "sys/xdma.h"
#include "sys/xintc.h"
#include "sys/xcache.h"

#include "kernel_signal.h"

#define XDMA_IO_BUFFER              (0x80000000)

#define XDMA1_MM2S_IRQ              (3)
#define XDAM2_MM2S_IRQ              (9)
#define XDAM2_S2MM_IRQ              (10)

#define XDMA0_BASE                  (0x41E00000)
#define XDMA1_BASE                  (0x41E10000)
#define XDAM2_BASE                  (0x41E20000)

#define XDMA1                       ((xsgdma_t *) XDMA1_BASE)
#define XDMA2                       ((xsgdma_t *) XDAM2_BASE)

#define XDMA0                       ((xdirectdma_t *) XDMA0_BASE)

#define XDMA0_MM2S                  ((xdirectdma_ch_t *) XDMA0_BASE)
#define XDMA0_S2MM                  ((xdirectdma_ch_t *) (XDMA0_BASE + 0x30))

#define XDMA_DATA_WIDTH             (32)
#define XDMA_MAX_BTT_WIDTH          (14)

#define XDMA_BTT_MAX                ((1ul << XDMA_MAX_BTT_WIDTH) - 1)
#define XDMA_IO_BUFFER_SIZE         ((1ul << XDMA_MAX_BTT_WIDTH))

/* ----------------------------------------------------------------------------------- */

#define XDMA0_IO_BUFFER             (XDMA_IO_BUFFER)

/* ----------------------------------------------------------------------------------- */

#define XDMA1_MAX_BDS               (2)

#define XDMA1_IO_BUFFER             (XDMA0_IO_BUFFER + XDMA_IO_BUFFER_SIZE)
#define XDMA1_IO_BD_RING            (XDMA1_IO_BUFFER + XDMA_IO_BUFFER_SIZE)

/* XDMA control register */

union xdmacr_u {

    uint32_t reg;

    struct {

        uint32_t rs               : 1;  /* Bit 0 : R/W  Run/Stop control (1 = run, 0 = stop) */
        uint32_t reserved1        : 1;  /* Bit 1 : RO   Reserved, always reads as 1 */
        uint32_t reset            : 1;  /* Bit 2 : R/W  Soft reset (self-clearing, resets entire DMA) */
        uint32_t keyhole          : 1;  /* Bit 3 : R/W  Keyhole read (fixed-address AXI reads) */
        uint32_t cyclic_bd_enable : 1;  /* Bit 4 : R/W  Enable cyclic BD mode (SG only) */
        uint32_t reserved5_11     : 7;  /* Bits 5–11 : RO Reserved, always read as 0 */
        uint32_t ioc_irq_en       : 1;  /* Bit 12 : R/W Interrupt-on-completion enable */
        uint32_t dly_irq_en       : 1;  /* Bit 13 : R/W Delay timer interrupt enable */
        uint32_t err_irq_en       : 1;  /* Bit 14 : R/W Error interrupt enable */
        uint32_t reserved15       : 1;  /* Bit 15 : RO Reserved, always reads as 0 */
        uint32_t irq_thrshld      : 8;  /* Bits 16–23 : R/W Interrupt threshold (min = 1, SG only) */
        uint32_t irq_delay        : 8;  /* Bits 24–31 : R/W Interrupt delay timeout (0 = disabled, SG only) */
    };
};

/* XDMA status register */

union xdmasr_u {

    uint32_t reg;

    struct {

        uint32_t halted           : 1;  /* Bit 0: DMA Channel Halted */
        uint32_t idle             : 1;  /* Bit 1: DMA Channel Idle */
        uint32_t reserved2        : 1;  /* Bit 2: Reserved, reads as 0 */
        uint32_t sg_incld         : 1;  /* Bit 3: Scatter Gather Included */
        uint32_t dma_int_err      : 1;  /* Bit 4: DMA Internal Error */
        uint32_t dma_slv_err      : 1;  /* Bit 5: DMA Slave Error */
        uint32_t dma_dec_err      : 1;  /* Bit 6: DMA Decode Error */
        uint32_t reserved7        : 1;  /* Bit 7: Reserved, reads as 0 */
        uint32_t sg_int_err       : 1;  /* Bit 8: SG Internal Error */
        uint32_t sg_slv_err       : 1;  /* Bit 9: SG Slave Error */
        uint32_t sg_dec_err       : 1;  /* Bit 10: SG Decode Error */
        uint32_t reserved11       : 1;  /* Bit 11: Reserved, reads as 0 */
        uint32_t ioc_irq          : 1;  /* Bit 12: Interrupt on Complete (W1C) */
        uint32_t dly_irq          : 1;  /* Bit 13: Interrupt on Delay (W1C) */
        uint32_t err_irq          : 1;  /* Bit 14: Interrupt on Error (W1C) */
        uint32_t reserved15       : 1;  /* Bit 15: Reserved, reads as 0 */
        uint32_t irq_thrshld_sts  : 8;  /* Bits 16–23: IRQ Threshold Status */
        uint32_t irq_delay_sts    : 8;  /* Bits 24–31: IRQ Delay Status */
    };
};

/* XDIRECTDMA length register */

union xdma_length_u {

    uint32_t reg;

    struct {
        uint32_t trans_len     : 26;  /* Bits 0–25  : R/W Transfer length in bytes.
                                       * Writing a non-zero value starts the MM2S transfer. */
        uint32_t reserved26_31 : 6;   /* Bits 26–31 : RO Reserved, always read as 0 */
    };
};

/* XSGDMA control register - used when miltichannel enabled */

union xdma_sgctl_u {

    uint32_t reg;

    struct {

        uint32_t sg_cache     : 4;  /* Bits 0–3  : R/W SG AXI cache control (ARCACHE/AWCACHE) */
        uint32_t reserved4_7  : 4;  /* Bits 4–7  : RO Reserved, always read as 0 */
        uint32_t sg_user      : 4;  /* Bits 8–11 : R/W SG AXI user control (ARUSER/AWUSER) */
        uint32_t reserved12_31: 20; /* Bits 12–31: RO Reserved, always read as 0 */
    };
};

/* XSGDMA descriptor ptr register */

struct xdma_desc_ptr_u {
    
    uint32_t lsb;                   /* Descriptor pointer lsb
                                     * Must be written only when DMA is halted.
                                     * Descriptor address must be 64-byte aligned. */

    uint32_t msb;                   /* Descriptor pointer msb */
};

/* XSGDMA scatter & gather descriptor */

struct xsgdmadesc_s {

    volatile struct {
        
        uint32_t lsb;
        uint32_t msb;
    } nextdesc;                       /* 0x00 - 0x04 */

    volatile struct {
        
        uint32_t lsb;
        uint32_t msb;
    } buffer_addr;                    /* 0x08 - 0x0C */
    
    volatile uint32_t reserved[2];    /* 0x10 - 0x14 */

    volatile struct {

        uint32_t buffer_len : 26;
        uint32_t tx_eof     : 1;
        uint32_t tx_sof     : 1;
        uint32_t reserved   : 4;
    } control;                        /* 0x18 */

    volatile struct {

        uint32_t trans_len     : 26;
        uint32_t reserved      : 2;
        uint32_t dma_intErr    : 1;
        uint32_t dma_intSlvErr : 1;
        uint32_t dma_decErr    : 1;
        uint32_t cmplt         : 1;
    } status;                         /* 0x1C */
    
    volatile uint32_t app0;           /* 0x20 */
    volatile uint32_t app1;           /* 0x24 */
    volatile uint32_t app2;           /* 0x28 */
    volatile uint32_t app3;           /* 0x2C */
    volatile uint32_t app4;           /* 0x30 */

    volatile uint32_t align[3];       /* 0x34 - 0x3f */
};

/* XDIRECTDMA channel */

struct xdirectdma_ch_s {

    volatile union xdmacr_u cr;     /* 0x00 + (ch_i * 0x30) */
    volatile union xdmasr_u sr;     /* 0x04 + (ch_i * 0x30)*/

    volatile uint32_t rsrv_0[4];    /* (0x08 - 0x14h) + (ch_i * 0x30) */

    volatile struct {

        uint32_t lsb;
        uint32_t msb;
    } m_addr;                       /* (0x18 - 0x1C) + (ch_i * 0x30) */

    volatile uint32_t hole[2];      /* (0x20 - 0x24) + (ch_i * 0x30) */
    volatile union xdma_length_u l; /* 0x28 + (ch_i * 0x30) */

    volatile uint32_t rsrv_1;       /* 0x2C + (ch_i * 0x30) */
};

/* XSGDMA channel */

struct xsgdma_ch_s {

    volatile union xdmacr_u cr;               /* 0x00 */
    volatile union xdmasr_u sr;               /* 0x04 */

    volatile struct xdma_desc_ptr_u currdesc; /* 0x08 - 0x0F */
    volatile struct xdma_desc_ptr_u taildesc; /* 0x10 - 0x17 */

    volatile uint32_t mm2s_hole[5];           /* 0x18 - 0x28 */

    volatile union xdma_sgctl_u sgctl;        /* 0x2C */
};

/* XDIRECTDMA memio instance */

struct xdirectdma_s {

    /* MM2S */
    volatile struct xdirectdma_ch_s mm2s; /* 0x00 - 0x2F */
    /* S2MM */
    volatile struct xdirectdma_ch_s s2mm; /* 0x30 - 0x5B */
};

/* XSGDMA memio instance */

struct xsgdma_s {

    /* MM2S */
    volatile struct xsgdma_ch_s mm2s; /* 0x00 - 0x2F */
    /* S2MM */
    volatile struct xsgdma_ch_s s2mm; /* 0x30 - 0x5B */
};

/* XDMA type definitions */

typedef union xdmacr_u xdmacr_t;
typedef union xdmasr_u xdmasr_t;

typedef union xdma_sgctl_u xdma_sgctl_t;
typedef union xdma_length_u xdma_length_t;
typedef struct xdma_desc_ptr_u xdma_desc_ptr_t;

typedef struct xsgdmadesc_s xsgdmadesc_t;

typedef struct xsgdma_s xsgdma_t;
typedef struct xsgdma_ch_s xsgdma_ch_t;

typedef struct xdirectdma_s xdirectdma_t;
typedef struct xdirectdma_ch_s xdirectdma_ch_t;

static volatile bool sgmm2s_cmpltSig;

static __attribute__((fast_interrupt)) void onxDMA1_irq(void);

// static bool xsgdma_trans(volatile xsgdma_ch_t *t_ch, volatile const uintptr_t mem, const size_t len);
static uintptr_t xdirectdma_trans(volatile xdirectdma_ch_t *t_ch, volatile const uintptr_t mem, const size_t len);

void _xdma_start(void) {

    /* reset both mm2s & s2mm engines */

    (XDMA0 -> mm2s.cr).reset = true;
    (XDMA0 -> s2mm.cr).reset = true;

    (XDMA1 -> mm2s.cr).reset = true;

    /* wait for reset done */

    while((XDMA0 -> mm2s.cr).reset) { }
    while((XDMA0 -> s2mm.cr).reset) { }

    while((XDMA1 -> mm2s.cr).reset) { }

    _xintc_enableIRQ(XDMA1_MM2S_IRQ, onxDMA1_irq);
}

volatile void const* _xdma_mm2s_simple(void const* mem, const size_t len) {

    /* Check if transfer is ongoing */
    if (!(XDMA0 -> mm2s.sr.halted) &&
            !(XDMA0 -> mm2s.sr.idle)) {
        return NULL;
    }

    /* check if transaction length doesnt exceed xdma data length register width */
    if ((mem == NULL) || (len == 0) || (len > XDMA_BTT_MAX)) {
        return NULL;
    }

    uintptr_t sa_addr = XDMA0_IO_BUFFER;
    uintptr_t mem_addr = (uintptr_t) mem;    

    _xdcache_invalidate(mem_addr, len);

    memcpy((void *) sa_addr, mem, len);
    _xdcache_flush(sa_addr, len);

    return (void *) xdirectdma_trans(XDMA0_MM2S, sa_addr, len);
}

volatile void const* _xdma_s2mm_simple(const size_t len) {

    /* check if transfer is ongoing */
    if (!(XDMA0 -> s2mm.sr.halted) &&
            !(XDMA0 -> s2mm.sr.idle)) {
        return NULL;
    }

    /* check if transaction length doesnt exceed xdma data length register width */
    if ((len == 0) || (len > XDMA_BTT_MAX)) {
        return NULL;
    }

    return (void *) xdirectdma_trans(XDMA0_S2MM, XDMA0_IO_BUFFER, len);
}

bool _xdma_mm2s_sgcmpltSignal(void) {
    return sgmm2s_cmpltSig;
}

void _xdma_mm2s_sgstop(void) {

    volatile xdmacr_t xdma_cr = { 0 };

    xdma_cr.rs = false;

    xdma_cr.reset = true;
    xdma_cr.ioc_irq_en = false;
    xdma_cr.err_irq_en = false;

    (XDMA1 -> mm2s.cr).reg = xdma_cr.reg;

    while((XDMA1 -> mm2s.cr).reset) { }
}

void * _xdma_mm2s_sgcmplt(const size_t len) {

    size_t i;

    xsgdmadesc_t *bd_desc;
    xsgdmadesc_t *start_desc = (xsgdmadesc_t *) XDMA1_IO_BD_RING;

    for (i = 0, bd_desc = start_desc; 
            (i < XDMA1_MAX_BDS) && (bd_desc != NULL) && (!bd_desc -> status.cmplt); 
                i++, bd_desc = (xsgdmadesc_t *) bd_desc -> nextdesc.lsb) {        
    }

    if ((i == XDMA1_MAX_BDS) || (bd_desc == NULL)) {
        return NULL;
    }

    size_t bd_blen = bd_desc -> control.buffer_len;
    uintptr_t bd_baddr = bd_desc -> buffer_addr.lsb;

    if (bd_blen != len) {
        return NULL;
    }

    bd_desc -> status.trans_len = 0;

    bd_desc -> status.cmplt = false;

    bd_desc -> status.dma_decErr = false;
    bd_desc -> status.dma_intErr = false;
    bd_desc -> status.dma_intSlvErr = false;

    sgmm2s_cmpltSig = false;

    return (void *) bd_baddr;
}

volatile void const* _xdma_mm2s_sgcyclic(const int seed, const size_t len, const size_t chunk) {

    /* check if input valid */
    if ((len > XDMA_IO_BUFFER_SIZE) ||
            (len < chunk) || ((len % chunk) != 0)) {
        return NULL;
    }

    xdmasr_t xdma_sr = { 0 };
    xdma_sr.reg = (XDMA1 -> mm2s.sr).reg;

    /*check if DMA1 in SG mode */
    if (!xdma_sr.sg_incld) {
        return NULL;
    }

    /* check if DMA1 not running */
    if (!(xdma_sr.halted) &&
            !(xdma_sr.idle)) {
        return NULL;
    }

    uintptr_t sa_addr = XDMA1_IO_BUFFER;

    memset((void *) sa_addr, seed, len);
    _xdcache_flush(sa_addr, len);

    /* setup cyclic bd ring */

    size_t bd_count = (len / chunk);
    size_t bd_size = bd_count * sizeof(xsgdmadesc_t);

    memset((void *) XDMA1_IO_BD_RING, 0, bd_size);

    size_t i;

    xsgdmadesc_t *bd_desc;
    xsgdmadesc_t *start_desc = (xsgdmadesc_t *) XDMA1_IO_BD_RING;    

    for (i = 0, bd_desc = start_desc; 
            i < bd_count; i++, sa_addr += chunk, bd_desc++) {

        bd_desc -> control.tx_sof = true;
        bd_desc -> control.tx_eof = true;

        bd_desc -> control.buffer_len = chunk;

        bd_desc -> buffer_addr.lsb = sa_addr;
        bd_desc -> nextdesc.lsb = (uint32_t) (bd_desc + 1);
    }

    bd_desc--;
    bd_desc -> nextdesc.lsb = (uint32_t) start_desc;

    _xdcache_flush(XDMA1_IO_BD_RING, bd_size);

    /* start DMA1 engine */

    sgmm2s_cmpltSig = false;

    XDMA1 -> mm2s.currdesc.lsb = (uint32_t) start_desc;    

    volatile xdmacr_t sgdma_cr = { 0 };

    sgdma_cr.rs = true;
    sgdma_cr.ioc_irq_en = true;
    sgdma_cr.err_irq_en = true;

    sgdma_cr.cyclic_bd_enable = true;

    (XDMA1 -> mm2s.cr).reg = sgdma_cr.reg;
    (XDMA1 -> mm2s.taildesc).lsb = 0x50;

    return (void *) sa_addr;
}

static uintptr_t xdirectdma_trans(volatile xdirectdma_ch_t *t_ch, volatile const uintptr_t addr, const size_t len) {

    /* check memory address is aligned at DMA words boundary */
    if (addr & (XDMA_DATA_WIDTH - 1)) {
        return 0;
    }

    t_ch -> m_addr.lsb = addr;

    t_ch -> cr.rs = true;
    t_ch -> l.trans_len = len;

    return addr;
}

/* DMA1 ISR */

static void onxDMA1_irq(void) {

    volatile xdmasr_t xdma_sr = { 0 };
    xdma_sr.reg = (XDMA1 -> mm2s.sr).reg;

    /* its either an error or i/o complete IRQ */

    bool ioe = xdma_sr.err_irq;
    bool ioc = xdma_sr.ioc_irq;

    (XDMA1 -> mm2s.sr).reg = xdma_sr.reg;

    int sgl = SIGINT;

    if (ioe) {
        sgl = SIGBUS;        
    }

    if (ioc) {
        sgmm2s_cmpltSig = true;
    }

    _kernel_raise(sgl);
}