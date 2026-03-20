#include <stdint.h>
#include <stdbool.h>

#include "sys/xintc.h"
#include "sys/xtimer.h"

#define XTIMER_IRQ                        (6)

#define XTIMER_FREQ                       (100000000)
#define XTIMER_MS_FREQ                    (XTIMER_FREQ / 1000)

#define XTIMER_BASE                       (0x41C00000)

#define XTIMER                            ((xtimer_t *) XTIMER_BASE)

#include <stdint.h>

typedef union {

    uint32_t reg;

    struct {

        uint32_t MDT     : 1;  // Bit 0: Timer Mode (0=Generate,1=Capture)
        uint32_t UDT     : 1;  // Bit 1: Up/Down Count Timer (0=Up,1=Down)
        uint32_t GENT    : 1;  // Bit 2: Enable External Generate Signal Timer
        uint32_t CAPT    : 1;  // Bit 3: Enable External Capture Trigger Timer
        uint32_t ARHT    : 1;  // Bit 4: Auto Reload/Hold Timer 
        uint32_t LOAD    : 1;  // Bit 5: Load Timer 
        uint32_t ENIT    : 1;  // Bit 6: Enable Interrupt for Timer 
        uint32_t ENT     : 1;  // Bit 7: Enable Timer 
        uint32_t TINT    : 1;  // Bit 8: Timer Interrupt Flag
        uint32_t PWMA    : 1;  // Bit 9: PWM Enable Timer 
        uint32_t ENALL   : 1;  // Bit 10: Enable All Timers
        uint32_t CASC    : 1;  // Bit 11: Cascade Mode
        uint32_t reserved: 20; // Bits 12-31: Reserved
    };
} xtimer_tcsr_t;

typedef struct {

    volatile xtimer_tcsr_t csr;  /* 0x00 - Timer Control/Status register */
    volatile uint32_t      lr;   /* 0x04 - Timer Load register */
    volatile uint32_t      cntr; /* 0x08 - Timer Counter register */
} xtimer_ch_t;

typedef struct {

    xtimer_ch_t tc0;      /* 0x00 */
    uint32_t    reserved; /* 0x0C */
    xtimer_ch_t tc1;      /* 0x10 */
} xtimer_t;

static __attribute__((fast_interrupt)) void timer_irq(void); 

void _xtimer_start(void) {

    (XTIMER -> tc0).csr.ENT = false;

    (XTIMER -> tc0).lr = 0;
    (XTIMER -> tc0).csr.LOAD = true;

    xtimer_tcsr_t tcsr = { 0 };

    tcsr.ENT = true;
    tcsr.ARHT = true;
    tcsr.ENIT = true;

    /* enable timer counter 0 channel */
    (XTIMER -> tc0).csr.reg = tcsr.reg;

    /* enable timer IRQ */
    _xintc_enableIRQ(XTIMER_IRQ, timer_irq);
}

uint32_t _xtimer_ticks(void) {
    return (XTIMER -> tc0).cntr;
}

uint32_t _xtimer_millis(void) {

    uint32_t ticks = _xtimer_ticks();
    uint32_t millis = (ticks / XTIMER_MS_FREQ);

    return millis;
}

/* xtimer isr */

static void timer_irq(void) {
    
    (XTIMER -> tc0).csr.TINT = true; /* ack timer irq */
}