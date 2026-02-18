#include <stddef.h>

#include "command.h"
#include "service.h"

#include "sys/rtmsure.h"

#include "kernel_stdio.h"
#include "kernel_signal.h"

static int rt_msuretime = -1;

static void rtmsure_service(void);
static int rtmsure_m(const int argc, const char **argv);

_SHELL_COMMAND(rtmsure, rtmsure_m);
_SERVICE(rtmsure_svc, rtmsure_service);

static void rtmsure_service(void) {

    sigset_t set;

    _kernel_sigemptyset(&set);
    _kernel_sigaddset(&set, SIGINT);
    _kernel_sigprocmask(SIG_BLOCK, &set, NULL);

    _kernel_sigpending(&set);

    if (!_kernel_sigismember(&set, SIGINT)) {
        return;
    }

    int time = _rtmsure_time();

    if (time == -1) {
        return;
    }

    rt_msuretime = time;
    _kernel_sigprocmask(SIG_UNBLOCK, &set, NULL);
}

static int rtmsure_m(const int argc, const char **argv) {

    (void) argc;
    (void) argv;

    if (rt_msuretime == -1) {

        _kernel_outString("no reaction time available\n");
        return -1;
    }

    if (rt_msuretime < 0) {
        
        _kernel_outString("unexpected reaction time format");
        return -1;
    }

    _kernel_outString("reaction time\n----------\n");    
    _kernel_outStringFormat("short: %d ms\nlong: %d ms\n", rt_msuretime, (int) 0);

    return 0;
}