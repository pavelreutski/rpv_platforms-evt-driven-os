#include <stdbool.h>

#include "uart.h"
#include "events.h"
#include "kernel.h"

#include "private/service.h"

static void uart_service(void);
static inline bool is_charValid(char c);

_KERNEL_SERVICE(uart_svc, uart_service);

static void uart_service(void) {

    char c;
    while((c = _uart_read()) != '\0') {

        if (is_charValid(c)) {

            console_key_t key = { .code = c, .scan_code = 0 };
            _kernel_pubEvt(EVT_USERCON_KEY, (evt_data_t *) &key);
        }
    }
}

static inline bool is_charValid(char c) {

    return ((c >= '0') && (c <= '9')) || 
                ((c >= 'a') && (c <= 'z')) ||
                ((c >= 'A') && (c <= 'Z')) ||
                ((c == ' ') || (c == '\n') || (c == '\t') || (c == '\b') || (c == '\x7f'));
}