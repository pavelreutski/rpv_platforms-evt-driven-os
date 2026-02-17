#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

#include "sys/xintc.h"

#define XINTC_BASE              (0x41200000)

#define XINTC                   ((xintc_t *) XINTC_BASE)

#define XINTC_INT(nIrq)         (1ul << (nIrq))

union xintc_mer_u {

    uint32_t reg;

    struct {

        uint32_t me           : 1;  /* Master IRQ Enable 0=Irq disabled - All interrupts disabled. 1=Irq enabled - All interrupts can be enabled */
        uint32_t hie          : 1;  /* Hardware Interrupt Enable 0=Read - Generating HW interrupts from SW enabled. Write - No effect. 1=Read - HW interrupts enabled. Write - Enable HW interrupts */
        uint32_t reserved2_31 : 29; /* Reserved */
    } bits;
};

struct xintc_s {

    volatile uint32_t           isr;       /* 0x00 - interrupt status register */
    volatile uint32_t           ipr;       /* 0x04 - interrupt pending register */
    volatile uint32_t           ier;       /* 0x08 - interrupt enable register */
    volatile uint32_t           iar;       /* 0x0c - interrupt acknowledge register */
    volatile uint32_t           sie;       /* 0x10 - set interrupt enable */
    volatile uint32_t           cie;       /* 0x14 - clear interrupt enable */
    volatile uint32_t           ivr;       /* 0x18 - interrupt vector register */
    volatile union xintc_mer_u  mer;       /* 0x1c - master enable register */
    volatile uint32_t           imr;       /* 0x20 - interrupt mode register */
    volatile uint32_t           ilr;       /* 0x24 - interrupt level register */
    volatile uint32_t           resrv[54]; /* 0x28 - reserved */
    volatile uint32_t           ivar[32];  /* 0x100 to 0x17c - interrupt vector address register */
};

typedef struct xintc_s xintc_t;
typedef union xintc_mer_u xintc_mer_t;

void _xintc_start(void) {

    volatile uint32_t isr = (XINTC -> isr);

    if (isr != 0) {
        (XINTC -> iar) = isr;
    }

    volatile xintc_mer_t l_mer = { 0 };

    l_mer.bits.me = true;
    l_mer.bits.hie = true;

    (XINTC -> mer.reg) = l_mer.reg; /* master enable interrupts */
}

void _xintc_disableIRQ(const uint8_t irq) {

    XINTC -> cie = XINTC_INT(irq);
    while(((XINTC -> ier) & XINTC_INT(irq))) { }  /* await irq disabled */
}

void _xintc_enableIRQ(const uint8_t irq, void (*const isr)(void)) {

    XINTC -> cie = XINTC_INT(irq);                /* disable IRQ */
    while(((XINTC -> ier) & XINTC_INT(irq))) { }  /* await irq disabled */

    (XINTC -> imr) |= XINTC_INT(irq);             /* set fast interrupt mode */

    uint32_t isr_addr = (uint32_t) isr;

    if (isr_addr != 0) {
        XINTC -> ivar[irq] = isr_addr;            /* set isr address */
    } else {
        isr_addr = XINTC -> ivar[irq];            /* read isr address */
    }

    if (isr_addr != 0) {

        XINTC -> sie = XINTC_INT(irq);                /* enable IRQ */
        while(!((XINTC -> ier) & XINTC_INT(irq))) { } /* await irq enabled */
    }
}