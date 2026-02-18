#include "command.h"
#include "service.h"

#include "sys/tsensor.h"

#include "kernel_stdio.h"

#define T_K                         (0.0625f)

static volatile uint8_t ts_temp     = 0;

static void temp_service(void);
static int temp_m(const int argc, const char **argv);

_SHELL_COMMAND(temp, temp_m);
_SERVICE(temp_svc, temp_service);

void temp_service(void) {

    uint16_t ts;
    if (!_tsensor_read(&ts)) {

        ts_temp = 0;
        return;
    }

    ts_temp = (uint8_t)(ts * T_K);
}

static int temp_m(const int argc, const char **argv) {

    (void) argc;
    (void) argv;

    if (ts_temp == 0) {

        _kernel_outString("No temperature data\n");
        return -1;
    }

    _kernel_outStringFormat("Temperature %d deg C\n", ts_temp);
    return 0;
}