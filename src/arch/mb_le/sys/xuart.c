#include <stdint.h>
#include <stdbool.h>

#include "sys/xuart.h"
#include "sys/xintc.h"

#define XUART_RX_BUFFER         (8)

#define XUART0_IRQ              (0)

#define XUART0_BASE             (0x40600000)
#define XUART1_BASE             (0x40610000)

#define XUART0                  ((xuartlite_t *) XUART0_BASE)
#define XUART1                  ((xuartlite_t *) XUART1_BASE)

/* Control register */

union xuartlite_ctrl_u {

    uint32_t reg;

    struct {

        uint32_t rst_txfifo   : 1;  /* Bit 0: Reset/clear transmit FIFO */
        uint32_t rst_rxfifo   : 1;  /* Bit 1: Reset/clear receive FIFO */
        uint32_t reserved2_3  : 2;  /* Bits 2-3: Reserved */
        uint32_t enable_irq   : 1;  /* Bit 4: Enable interrupt */
        uint32_t reserved5_31 : 27; /* Bits 5-31: Reserved */
    };
};

/* Status register */

union xuartlite_status_u {

    uint32_t reg;
    
    struct {

        uint32_t rxfifo_valid   : 1;  /* Bit 0: 1 = data in receive FIFO */
        uint32_t rxfifo_full    : 1;  /* Bit 1: 1 = receive FIFO full */
        uint32_t txfifo_empty   : 1;  /* Bit 2: 1 = transmit FIFO empty */
        uint32_t txfifo_full    : 1;  /* Bit 3: 1 = transmit FIFO full */
        uint32_t irq_enabled    : 1;  /* Bit 4: interrupts enabled */
        uint32_t ovr_error      : 1;  /* Bit 5: overrun error */
        uint32_t frame_error    : 1;  /* Bit 6: frame error */
        uint32_t parity_error   : 1;  /* Bit 7: parity error */
        uint32_t reserved8_31   : 24; /* Bits 8-31 reserved */
    };
};

struct xuartlite_s {

    volatile uint32_t rx_datafifo;         /* 0x00 */
    volatile uint32_t tx_datafifo;         /* 0x04 */

    volatile union xuartlite_status_u sr; /* 0x08 */
    volatile union xuartlite_ctrl_u cr;   /* 0x0c */
};

typedef struct xuartlite_s xuartlite_t;

typedef union xuartlite_ctrl_u xuartlite_ctrl_t;
typedef union xuartlite_status_u xuartlite_status_t;

static volatile uint8_t rx0_tail;
static volatile uint8_t rx0_head;

static volatile uint8_t rx0_ring[XUART_RX_BUFFER];

static __attribute__((fast_interrupt)) void onxUart0_irq(void);

void _xuartlite_start(void) {

    _xintc_disableIRQ(XUART0_IRQ);
    _xintc_enableIRQ(XUART0_IRQ, onxUart0_irq);

    volatile xuartlite_ctrl_t uart_cr = { 0 };

    uart_cr.rst_rxfifo = true;
    uart_cr.rst_txfifo = true;

    uart_cr.enable_irq = true;

    (XUART0 -> cr.reg) = uart_cr.reg;
}

char _xuartlite_read(void) {

    if (rx0_head == rx0_tail) {
        return '\0';
    }

    uint8_t rx_head = rx0_head;
    char c = (char) rx0_ring[rx_head];

    rx_head++;
    if (rx_head == XUART_RX_BUFFER) {
        rx_head = 0;
    }

    rx0_head = rx_head;

    return (char) c;
}

void _xuartlite_write(const char c) {

    while (!(XUART0 -> sr.txfifo_empty)) { }
    XUART0 -> tx_datafifo = c;
}

static void onxUart0_irq(void) {

    if (!(XUART0 -> sr.rxfifo_valid)) {
        return;
    }

    do {

        volatile uint8_t c = 
            (uint8_t) ((XUART0 -> rx_datafifo) & 0xFF);

        uint8_t next_tail = (rx0_tail + 1);
        if (next_tail == XUART_RX_BUFFER) {
            next_tail = 0;
        }

        if (next_tail == rx0_head) {

            rx0_head++;
            if (rx0_head == XUART_RX_BUFFER) {
                rx0_head = 0;
            }
        }

        rx0_ring[rx0_tail] = c;
        rx0_tail = next_tail;

    } while((XUART0-> sr.rxfifo_valid));
}