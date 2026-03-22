#include "command.h"
#include "service.h"

#include "kernel_stdio.h"

#include "sys/xtimer.h"

#define WAIT_TIME_MILLIS               (5000)

static void timer_service(void);
static int systime_m(const int argc, const char** argv);

_SHELL_COMMAND(systime, systime_m);
_SERVICE(timer_svc, timer_service);

static void timer_service(void) {

}

static int systime_m(const int argc, const char** argv) {

    (void) argc;
    (void) argv;

    _kernel_outStringFormat("time since system startup: %d ms\n", (int)_xtimer_millis());
    _kernel_outStringFormat("wait %d milliseconds...\n", (int)WAIT_TIME_MILLIS);
    
    volatile uint32_t end_time = (_xtimer_millis() + WAIT_TIME_MILLIS);
    while(end_time > _xtimer_millis()) { } // wait

    return 0;
}