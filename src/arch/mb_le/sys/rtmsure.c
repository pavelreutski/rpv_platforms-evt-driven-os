#include <stdbool.h>

#include "sys/xintc.h"
#include "sys/rtmsure.h"

#include "kernel_signal.h"

#define RTMSURE_IRQ                     (4)
#define RTMSURE_BASE                    (0x44A30000)

#define RTMSURE                         ((rtmsure_t *) RTMSURE_BASE)

union rtmsure_status_u {

    uint32_t reg;
    struct {
        uint32_t irq_pending : 1; /* BIT 0: IRQ pending bit */
    }; 
};

struct rtmsure_s {

        volatile uint32_t dtr;              /* 0x00 - Data register */
        volatile union rtmsure_status_u sr; /* 0x04 - Status register */
};

typedef struct rtmsure_s rtmsure_t;
typedef union rtmsure_status_u rtmsure_status_t;

static volatile int bcd_time;

static int bcd_toDec(const int bcd_time);

static __attribute__((fast_interrupt)) void onRtmsure_irq(void);

void _rtmsure_start(void) {

    bcd_time = -1;
    _xintc_enableIRQ(RTMSURE_IRQ, onRtmsure_irq);
}

int _rtmsure_time(void) {

    volatile int time = bcd_time;

    bcd_time = -1;
    if (time < 0) {
        return time;
    }

    time = bcd_toDec(time);
    return time;
}

static int bcd_toDec(const int bcd_time) {

    int dec = 0;
    int base = 1;

    int time = bcd_time;
    while (time > 0) {
        
        uint32_t dec_nth = (time & 0x0F);
        dec_nth *= base;

        base *= 10;
        dec += dec_nth;

        time >>= 4;
    }
    
    return dec;
}

static void onRtmsure_irq(void) {

    rtmsure_status_t irq_ack = { 0 };

    irq_ack.irq_pending = true;
    (RTMSURE -> sr).reg = irq_ack.reg;

    uint32_t dt = (RTMSURE -> dtr);
    bcd_time = (int) dt;

    _kernel_raise(SIGINT);
}