#include <stdint.h>
#include <string.h>

#include "sys/xdma.h"
#include "sys/xintc.h"
#include "sys/xcache.h"

#include "kernel_signal.h"

#define XDMA1_MM2S_IRQ              (3)
#define XDMA2_MM2S_IRQ              (9)
#define XDMA2_S2MM_IRQ              (10)

#define XDMA0_BASE                  (0x41E00000)
#define XDMA1_BASE                  (0x41E10000)
#define XDMA2_BASE                  (0x41E20000)

#define XDMA1                       ((xsgdma_t *) XDMA1_BASE)
#define XDMA2                       ((xsgdma_t *) XDMA2_BASE)

#define XDMA0                       ((xdirectdma_t *) XDMA0_BASE)

#define XDMA0_MM2S                  ((xdirectdma_ch_t *) XDMA0_BASE)
#define XDMA0_S2MM                  ((xdirectdma_ch_t *) (XDMA0_BASE + 0x30))

#define XDMA1_MM2S                  ((xsgdma_ch_t *) XDMA1_BASE)
#define XDMA1_S2MM                  ((xsgdma_ch_t *) (XDMA1_BASE + 0x30))

#define XDMA2_MM2S                  ((xsgdma_ch_t *) XDMA2_BASE)
#define XDMA2_S2MM                  ((xsgdma_ch_t *) (XDMA2_BASE + 0x30))

#define XDMA_DATA_WIDTH             (32)
#define XDMA_MAX_BTT_WIDTH          (14)

#define XDMA_DATA_ALIGN             ((XDMA_DATA_WIDTH / 8) - 1)

#define XDMA_BTT_MAX                ((1ul << XDMA_MAX_BTT_WIDTH) - 1)
#define XDMA_IO_BUFFER_SIZE         ((1ul << XDMA_MAX_BTT_WIDTH))

/* --------------------------------- XSGDMA BD --------------------------------------- */

#define XDMA1_MAX_BDS               (2)
#define XDMA2_MAX_BDS               (10)

/* --------------------------------- XDMA0 I/O --------------------------------------- */

#define XDMA0_IO_ADDR               (0x80000000)

/* --------------------------------- XSGDMA1 I/O ------------------------------------- */

#define XDMA1_IO_ADDR               (0x80004000) // 16K I/O space
#define XDMA1_MM2S_RINGADDR         (0x80010000) // 2 buffers circular queue

/* --------------------------------- XSGDMA2 I/O ------------------------------------- */

#define XDMA2_IO_ADDR               (0x80008000) // 32K I/O space

#define XDMA2_S2MM_RINGADDR         (0x80010080) // 10 buffers circular queue
#define XDMA2_MM2S_RINGADDR         (0x80010300) // 10 buffers circular queue

/* ----------------------------------------------------------------------------------- */

#define XDMA0_IO_BUFFER             ((void *) XDMA0_IO_ADDR)
#define XDMA1_IO_BUFFER             ((void *) XDMA1_IO_ADDR)

#define XDMA2_S2MM_BUFFER           ((void *) XDMA2_IO_ADDR)
#define XDMA2_MM2S_BUFFER           ((void *) (XDMA2_IO_ADDR + XDMA_IO_BUFFER_SIZE))

#define XDMA1_MM2S_BDRING           ((xsgdmadesc_t *) XDMA1_MM2S_RINGADDR)

#define XDMA2_S2MM_BDRING           ((xsgdmadesc_t *) XDMA2_S2MM_RINGADDR)
#define XDMA2_MM2S_BDRING           ((xsgdmadesc_t *) XDMA2_MM2S_RINGADDR)

/* XDMA control register */

typedef union xdmacr_u {

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
} xdmacr_t;

/* XDMA status register */

typedef union xdmasr_u {

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
} xdmasr_t;

/* XDIRECTDMA length register */

typedef union xdma_length_u {

    uint32_t reg;

    struct {
        uint32_t trans_len     : 26;  /* Bits 0–25  : R/W Transfer length in bytes.
                                       * Writing a non-zero value starts the MM2S transfer. */
        uint32_t reserved26_31 : 6;   /* Bits 26–31 : RO Reserved, always read as 0 */
    };
} xdma_length_t;

/* XSGDMA control register - used when miltichannel enabled */

typedef union xdma_sgctl_u {

    uint32_t reg;

    struct {

        uint32_t sg_cache     : 4;  /* Bits 0–3  : R/W SG AXI cache control (ARCACHE/AWCACHE) */
        uint32_t reserved4_7  : 4;  /* Bits 4–7  : RO Reserved, always read as 0 */
        uint32_t sg_user      : 4;  /* Bits 8–11 : R/W SG AXI user control (ARUSER/AWUSER) */
        uint32_t reserved12_31: 20; /* Bits 12–31: RO Reserved, always read as 0 */
    };
} xdma_sgctl_t;

/* XSGDMA scatter & gather descriptor */

typedef struct xsgdmadesc_s {

    volatile union {
        
        struct xsgdmadesc_s *next;

        struct {

            uint32_t lsb;
            uint32_t msb;
        } nextdesc_addr;
    };                                /* 0x00 - 0x04 */

    volatile union {
        
        void *buffer;

        struct {
            uint32_t lsb;
            uint32_t msb;
        } buffer_addr;
    };                                /* 0x08 - 0x0C */
    
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
} xsgdmadesc_t;

/* XDIRECTDMA channel */

typedef struct xdirectdma_ch_s {

    volatile union xdmacr_u cr;     /* 0x00 + (ch_i * 0x30) */
    volatile union xdmasr_u sr;     /* 0x04 + (ch_i * 0x30)*/

    volatile uint32_t rsrv_0[4];    /* (0x08 - 0x14h) + (ch_i * 0x30) */

    volatile union {

        void *buffer;

        struct {

            uint32_t lsb;
            uint32_t msb;
        } buffer_addr;              /* (0x18 - 0x1C) + (ch_i * 0x30) */
    };

    volatile uint32_t hole[2];      /* (0x20 - 0x24) + (ch_i * 0x30) */
    volatile union xdma_length_u l; /* 0x28 + (ch_i * 0x30) */

    volatile uint32_t rsrv_1;       /* 0x2C + (ch_i * 0x30) */
} xdirectdma_ch_t;

/* XSGDMA channel */

typedef struct xsgdma_ch_s {

    volatile union xdmacr_u cr;               /* 0x00 */
    volatile union xdmasr_u sr;               /* 0x04 */

    volatile union {

        struct xsgdmadesc_s *currdesc;

        struct {

            uint32_t lsb;
            uint32_t msb;
        } currdesc_addr;
    };                                        /* 0x08 0x0F */

    volatile union {

        struct xsgdmadesc_s *taildesc;

        struct {

            uint32_t lsb;
            uint32_t msb;
        } taildesc_addr;                      /* 0x10 - 0x17 */
    };

    volatile uint32_t mm2s_hole[5];           /* 0x18 - 0x28 */
    volatile union xdma_sgctl_u sgctl;        /* 0x2C */
} xsgdma_ch_t;

/* XDIRECTDMA memio instance */

typedef struct xdirectdma_s {

    /* MM2S */
    volatile struct xdirectdma_ch_s mm2s; /* 0x00 - 0x2F */
    /* S2MM */
    volatile struct xdirectdma_ch_s s2mm; /* 0x30 - 0x5B */
} xdirectdma_t;

/* XSGDMA memio instance */

typedef struct xsgdma_s {

    /* MM2S */
    volatile struct xsgdma_ch_s mm2s; /* 0x00 - 0x2F */
    /* S2MM */
    volatile struct xsgdma_ch_s s2mm; /* 0x30 - 0x5B */
} xsgdma_t;

/* XSGDMA signals */

typedef struct xsgdma_sig_s {

    volatile bool sgdma1_mm2sCmpltSig  : 1; /* Bit 0: SGDMA1 MM2S transaction completed */
    volatile bool sgdma1_mm2sBusErrSig : 1; /* Bit 1: SGDMA1 MM2S bus error */

    volatile bool sgdma2_mm2sCmpltSig  : 1; /* Bit 2: SGDMA2 MM2S transaction completed */
    volatile bool sgdma2_mm2sBusErrSig : 1; /* Bit 3: SGDMA2 MM2S bus error */

    volatile bool sgdma2_s2mmCmpltSig  : 1; /* Bit 4: SGDMA2 S2MM transaction completed */
    volatile bool sgdma2_s2mmBusErrSig : 1; /* Bit 5: SGDMA2 S2MM bus error */
} xsgdma_sigset_t;

static volatile xsgdma_sigset_t sgdma_sgls;

static __attribute__((fast_interrupt)) void onxDMA1_irq(void);

static __attribute__((fast_interrupt)) void onxDMA2_mm2sIrq(void);
static __attribute__((fast_interrupt)) void onxDMA2_s2mmIrq(void);

static xdmasr_t xsgdma_ackIrq(volatile xsgdma_ch_t *t_ch);

static inline bool xsgdma_cyclicAllowed(volatile xsgdma_ch_t *t_ch);

static void xsgdma_engstop(volatile xsgdma_ch_t *t_ch);
static void xsgdma_cyclicTrans(volatile xsgdma_ch_t *t_ch, xsgdmadesc_t const* head);
static void xsgdma_cyclicRing(xsgdmadesc_t *const head, void *const io_head, const size_t len, const size_t chunk);

// static bool xsgdma_trans(volatile xsgdma_ch_t *t_ch, volatile const uintptr_t mem, const size_t len);
static void *xdirectdma_trans(volatile xdirectdma_ch_t *t_ch, void const* mem, const size_t len);

void _xdma_start(void) {

    /* disable DMA engines IRQ */

    _xintc_disableIRQ(XDMA1_MM2S_IRQ);

    _xintc_disableIRQ(XDMA2_MM2S_IRQ);
    _xintc_disableIRQ(XDMA2_S2MM_IRQ);

    /* reset both mm2s & s2mm engines for all DMA instances */

    (XDMA0_MM2S -> cr).reset = true;
    (XDMA0_S2MM -> cr).reset = true;

    (XDMA1_MM2S -> cr).reset = true;

    /* wait for engines reset done */

    while((XDMA0_MM2S -> cr).reset) { }
    while((XDMA0_S2MM -> cr).reset) { }

    while((XDMA1_MM2S -> cr).reset) { }

    /* enable DMA engines IRQ */

    _xintc_enableIRQ(XDMA1_MM2S_IRQ, onxDMA1_irq);

    _xintc_enableIRQ(XDMA2_MM2S_IRQ, onxDMA2_mm2sIrq);
    _xintc_enableIRQ(XDMA2_S2MM_IRQ, onxDMA2_s2mmIrq);
}

void _xdma2_mm2s_sgstop(void) {
    xsgdma_engstop(XDMA2_MM2S);
}

void _xdma2_s2mm_sgstop(void) {
    xsgdma_engstop(XDMA2_S2MM);
}

void _xdma1_mm2s_sgstop(void) {
    xsgdma_engstop(XDMA1_MM2S);
}

bool _xdma1_mm2s_sgbuserrSignal(void) {
    return sgdma_sgls.sgdma1_mm2sBusErrSig;
}

bool _xdma2_s2mm_sgcmpltSignal(void) {
    return sgdma_sgls.sgdma2_s2mmCmpltSig;
}

bool _xdma1_mm2s_sgcmpltSignal(void) {
    return sgdma_sgls.sgdma1_mm2sCmpltSig;
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

    _xdcache_invalidate((uintptr_t) mem, len);

    memcpy(XDMA0_IO_BUFFER, mem, len);
    _xdcache_flush(XDMA0_IO_ADDR, len);

    return xdirectdma_trans(XDMA0_MM2S, XDMA0_IO_BUFFER, len);
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

    return xdirectdma_trans(XDMA0_S2MM, XDMA0_IO_BUFFER, len);
}

void *_xdma2_s2mm_sgcmplt(void *mem, const size_t mem_len, size_t *const trans_len) {

    if ((mem == NULL) || (trans_len == NULL)) {
        return NULL;
    }

    _xdcache_invalidate(XDMA2_S2MM_RINGADDR, sizeof(xsgdmadesc_t) * XDMA2_MAX_BDS);

    size_t i;

    xsgdmadesc_t *bd_desc;
    xsgdmadesc_t *start_desc = XDMA2_S2MM_BDRING;

    for (i = 0, bd_desc = start_desc; 
            (i < XDMA2_MAX_BDS) && (bd_desc != NULL) && (!bd_desc -> status.cmplt); 
                i++, bd_desc = bd_desc -> next) {        
    }

    if ((i == XDMA2_MAX_BDS) || (bd_desc == NULL)) {
        return NULL;
    }

    void *bd_buffer = bd_desc -> buffer;
    size_t bd_translen = bd_desc -> status.trans_len;

    bd_desc -> status.trans_len = 0;
    bd_desc -> status.cmplt = false;

    bd_desc -> status.dma_decErr = false;
    bd_desc -> status.dma_intErr = false;
    bd_desc -> status.dma_intSlvErr = false;

    if (bd_translen > mem_len) {
        return NULL;
    }

    _xdcache_flush((uintptr_t) bd_desc, sizeof(xsgdmadesc_t));
    _xdcache_invalidate((uintptr_t) bd_buffer, bd_translen);

    memcpy(mem, bd_buffer, bd_translen);

    *trans_len = bd_translen;

    sgdma_sgls.sgdma2_s2mmCmpltSig = false;
    return bd_buffer;
}

void *_xdma1_mm2s_sgcmplt(const size_t len) {

    _xdcache_invalidate(XDMA1_MM2S_RINGADDR, sizeof(xsgdmadesc_t) * XDMA1_MAX_BDS);

    size_t i;

    xsgdmadesc_t *bd_desc;
    xsgdmadesc_t *start_desc = XDMA1_MM2S_BDRING;

    for (i = 0, bd_desc = start_desc; 
            (i < XDMA1_MAX_BDS) && (bd_desc != NULL) && (!bd_desc -> status.cmplt); 
                i++, bd_desc = bd_desc -> next) {        
    }

    if ((i == XDMA1_MAX_BDS) || (bd_desc == NULL)) {
        return NULL;
    }

    void *bd_buffer = bd_desc -> buffer;
    size_t bd_blen = bd_desc -> control.buffer_len;

    if (bd_blen != len) {
        return NULL;
    }

    bd_desc -> status.trans_len = 0;

    bd_desc -> status.cmplt = false;

    bd_desc -> status.dma_decErr = false;
    bd_desc -> status.dma_intErr = false;
    bd_desc -> status.dma_intSlvErr = false;

    _xdcache_flush((uintptr_t) bd_desc, sizeof(xsgdmadesc_t));

    sgdma_sgls.sgdma1_mm2sCmpltSig = false;
    return bd_buffer;
}

volatile void const* _xdma1_mm2s_sgcyclic(const int seed, const size_t len, const size_t chunk) {

    /* check if input valid */
    if (((chunk & XDMA_DATA_ALIGN) != 0) || 
            (len > XDMA_IO_BUFFER_SIZE) ||
                (len < chunk) || ((len % chunk) != 0)) {
        return NULL;
    }

    if (!xsgdma_cyclicAllowed(XDMA1_MM2S)) {
        return NULL;
    }

    (XDMA1_MM2S -> cr).reset = true;
    while((XDMA1_MM2S -> cr).reset) { } // wait reset done

    memset(XDMA1_IO_BUFFER, seed, len);
    _xdcache_flush(XDMA1_IO_ADDR, len);

    /* setup cyclic bd ring */
    xsgdma_cyclicRing(XDMA1_MM2S_BDRING, XDMA1_IO_BUFFER, len, chunk);

    /* start DMA1 engine */

    sgdma_sgls.sgdma1_mm2sCmpltSig = false;
    sgdma_sgls.sgdma1_mm2sBusErrSig = false;

    xsgdma_cyclicTrans(XDMA1_MM2S, XDMA1_MM2S_BDRING);

    return XDMA1_IO_BUFFER;
}

volatile void const* _xdma2_s2mm_sgcyclic(const size_t len, const size_t chunk) {

    /* check if input valid */
    if (((chunk & XDMA_DATA_ALIGN) != 0) || 
            (len > XDMA_IO_BUFFER_SIZE) ||
                (len < chunk) || ((len % chunk) != 0)) {
        return NULL;
    }

    if (!xsgdma_cyclicAllowed(XDMA2_S2MM)) {
        return NULL;
    }

    /* setup cyclic s2mm bd ring */
    xsgdma_cyclicRing(XDMA2_S2MM_BDRING, XDMA2_S2MM_BUFFER, len, chunk);

    /* start DMA2 S2MM engine */

    sgdma_sgls.sgdma2_s2mmCmpltSig  = false;
    sgdma_sgls.sgdma2_s2mmBusErrSig = false;

    xsgdma_cyclicTrans(XDMA2_S2MM, XDMA2_S2MM_BDRING);

    return XDMA2_S2MM_BUFFER;
}

/********************************************************************************************************************/

static inline bool xsgdma_cyclicAllowed(volatile xsgdma_ch_t *t_ch) {

    xdmasr_t xdma_sr = { 0 };
    xdma_sr.reg = (t_ch -> sr).reg;

    /*check if DMA1 in SG mode */
    volatile bool sgdmaIncl = xdma_sr.sg_incld;
    /* check if DMA1 not running */
    volatile bool sgdmaIdle = xdma_sr.halted || xdma_sr.idle;

    return (sgdmaIncl && sgdmaIdle);
}

static void xsgdma_engstop(volatile xsgdma_ch_t *t_ch) {

    volatile xdmacr_t xdma_cr = { 0 };

    xdma_cr.rs = false;
    xdma_cr.reset = true;
    
    xdma_cr.ioc_irq_en = false;
    xdma_cr.err_irq_en = false;

    (t_ch -> cr).reg = xdma_cr.reg;

    while ((t_ch -> cr).reset) { } // wait reset done
}

static void xsgdma_cyclicRing(xsgdmadesc_t *const head, void *const io_head, const size_t len, const size_t chunk) {

    /* setup cyclic bd ring */

    size_t bd_count = (len / chunk);
    size_t bd_memsize = bd_count * sizeof(xsgdmadesc_t);

    memset((void *) head, 0, bd_memsize);

    size_t i;
    
    xsgdmadesc_t *bd_desc;
    uintptr_t sa_addr = (uintptr_t) io_head;

    for (i = 0, bd_desc = head; 
            i < bd_count; i++, sa_addr += chunk, bd_desc++) {

        bd_desc -> control.tx_sof = true;
        bd_desc -> control.tx_eof = true;

        bd_desc -> control.buffer_len = chunk;

        bd_desc -> next = (bd_desc + 1);
        bd_desc -> buffer_addr.lsb = sa_addr;
    }

    bd_desc--;
    bd_desc -> next = head;

    _xdcache_flush((uintptr_t) head, bd_memsize);
}

static void xsgdma_cyclicTrans(volatile xsgdma_ch_t *t_ch, xsgdmadesc_t const* head) {   

    volatile xdmacr_t sgdma_cr = { 0 };

    sgdma_cr.rs = true;
    sgdma_cr.ioc_irq_en = true;
    sgdma_cr.err_irq_en = true;

    sgdma_cr.cyclic_bd_enable = true;

    (t_ch -> currdesc) = (xsgdmadesc_t *) head;

    (t_ch -> cr).reg = sgdma_cr.reg;
    (t_ch -> taildesc_addr).lsb = 0x50;    // begin DMA DRQ
}

static void* xdirectdma_trans(volatile xdirectdma_ch_t *t_ch, void const* mem, const size_t len) {

    /* check memory address is aligned at DMA words boundary */
    if (((uintptr_t) mem) & (XDMA_DATA_WIDTH - 1)) {
        return 0;
    }

    void *t_buffer = (void *) mem;

    t_ch -> buffer = t_buffer;

    t_ch -> cr.rs = true;
    t_ch -> l.trans_len = len;

    return t_buffer;
}

static xdmasr_t xsgdma_ackIrq(volatile xsgdma_ch_t *t_ch) {

    volatile xdmasr_t xdma_sr = { 0 };
    xdma_sr.reg = (t_ch -> sr).reg;

    /* ack DMA engine irq */
    (t_ch -> sr).reg = xdma_sr.reg;

    /* its either an error or i/o complete IRQ */
    volatile int sgl = xdma_sr.err_irq ? SIGBUS : SIGINT;

    _kernel_raise(sgl);
    return xdma_sr;
}

/* DMA1 ISR */

static void onxDMA1_irq(void) {

    xdmasr_t sr = xsgdma_ackIrq(XDMA1_MM2S);

    sgdma_sgls.sgdma1_mm2sCmpltSig = sr.ioc_irq;
    sgdma_sgls.sgdma1_mm2sBusErrSig = sr.err_irq; 
}

/* DMA2 ISRs */

static void onxDMA2_mm2sIrq(void) {

    xdmasr_t sr = xsgdma_ackIrq(XDMA2_MM2S);

    sgdma_sgls.sgdma2_mm2sCmpltSig = sr.ioc_irq;
    sgdma_sgls.sgdma2_mm2sBusErrSig = sr.err_irq;
}

static void onxDMA2_s2mmIrq(void) {

    xdmasr_t sr = xsgdma_ackIrq(XDMA2_S2MM);

    sgdma_sgls.sgdma2_s2mmCmpltSig = sr.ioc_irq;
    sgdma_sgls.sgdma2_s2mmBusErrSig = sr.err_irq;
}