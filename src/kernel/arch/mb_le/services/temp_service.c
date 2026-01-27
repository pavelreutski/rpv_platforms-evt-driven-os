#include "command.h"
#include "service.h"

#include "sys/tsensor.h"

#include "kernel_stdio.h"

#define T_K                         (0.0625f)

static volatile uint8_t ts_temp     = 0;

static void temp_service(void);
static uint8_t on_tempQuery(char const* input, const int argc, const char **argv);

_SERVICE(temp_svc, temp_service);
_SHELL_COMMAND(temp, on_tempQuery);

void temp_service(void) {

    uint16_t ts;
    if (!_tsensor_read(&ts)) {

        ts_temp = 0;
        return;
    }

    ts_temp = (uint8_t)(ts * T_K);
}

static uint8_t on_tempQuery(char const* input, const int argc, const char **argv) {

    (void) argc;
    (void) argv;

    (void) input;

    if (ts_temp == 0) {

        _kernel_outString("No temperature data\n");
        return EXEC_BUILT_IN;
    }

    _kernel_outStringFormat("Temperature %d deg C\n", ts_temp);
    return EXEC_BUILT_IN;
}