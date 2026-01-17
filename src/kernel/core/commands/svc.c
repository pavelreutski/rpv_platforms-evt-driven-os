#include <stdint.h>

#include "kernel_stdio.h"
#include "kernel_sevice.h"

#include "private/command.h"

static uint8_t onSvc_command(char const* input, const int argc, const char **argv);

_KERNEL_SHELL_COMMAND(svc, onSvc_command);

uint8_t onSvc_command(char const* input, const int argc, const char **argv) {
    
    (void) argc;
    (void) argv;

    (void) input;

    int svc_n = 0;

    void *svc = NULL;
    char const* svc_name = NULL;

    while ((svc = _kernel_service(svc, &svc_name)) != NULL) {

        svc_n++;
        _kernel_outStringFormat("%s\n", svc_name);
    }

    if(svc_n == 0) {
        _kernel_outString("no services\n");
    }

    return EXEC_BUILT_IN;
}