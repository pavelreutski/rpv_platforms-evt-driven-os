#include MCU_CONFIG

#include <string.h>

#include "spi.h"
#include "gpio.h"

#include "service.h"
#include "command.h"

#include "kernel_stdio.h"

static char const* whl_command(char *command, char const *argv);

static void whls_service(void);
static int whls_m(const int argc, const char** argv);

_SHELL_COMMAND(whls, whls_m);
_SERVICE(whl_svc, whls_service);

static void whls_service(void) { }

static int whls_m(const int argc, const char** argv) {

    (void) argc;
    (void) argv;

    if (argc < 2) {

        _kernel_outString("no wheel command given\n");
        return -1;
    }

    // whls cs:0
    // whls cs:1

    char cmd[4];
    char const* arg = whl_command(cmd, argv[1]);

    if (strcmp(cmd, "cs") == 0) {

        char c = *arg;

        switch (c) {

            case '0': {

                _kernel_outString("whlcnt sel\n");
                _gpiob_write(SPI_CS_PIN, false);

                return 0;
            } break;

            case '1': {

                _kernel_outString("whlcnt unsel\n");
                _gpiob_write(SPI_CS_PIN, true);

                return 0;
            } break;
        
            default: {
                _kernel_outString("unknown action\n");                
            } break;
        }
    }

    // whls cnt

    if (strcmp(cmd, "cnt") == 0) {

        int c = _spi_write(0);        
        _kernel_outStringFormat("whlcnt %d\n", c);
        
        return 0;
    }

    _kernel_outString("unknown command\n");
    return -1;
}

static char const* whl_command(char *command, char const* argv) {    

    char c;
    
    char *cmd;
    char const* arg;

    for (cmd = command, arg = (char *) argv, 
                c = *arg; c != '\0' && c != ':'; 
                    arg++, c = *arg, cmd++) {

        *cmd = c; 
    }

    *cmd = '\0';

    if (c == ':') {
        arg++;
    }

    return arg;
}