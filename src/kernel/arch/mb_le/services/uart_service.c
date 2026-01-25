#include "kernel.h"
#include "service.h"

#include "sys/xuart.h"

static void uart_service(void);

_SERVICE(uart_svc, uart_service);

static void uart_service(void) {

    char c;
    while ((c = _xuartlite_read()) != '\0') {

        console_key_t key = { .code = c, .scan_code = 0 };
        _kernel_pubEvt(EVT_USERCON_KEY, (evt_data_t *) &key);
    }
}