#include <stdlib.h>

#include "command.h"

#include "kernel_stdio.h"

static int readm32(char const* addr_s);
static int writem32(char const* addr_s, char const* val_s);

static int rwmon32_m(const int argc, const char** argv);

_SHELL_COMMAND(rwmon32, rwmon32_m);

static int rwmon32_m(const int argc, const char** argv) {

    if (argc < 2) {

        _kernel_outString("mem addr or size required");
        return -1;
    }

    if (argc == 2) {
        return readm32(argv[1]);
    }

    return writem32(argv[1], argv[2]);
}

static int readm32(char const* addr_s) {

    char *c;
    uint32_t *addr = (uint32_t *) strtoul(addr_s, &c, 16);

    if (*c != '\0') {

        _kernel_outString("mem addr invalid\n");
        return -1;
    }

    _kernel_outStringFormat("0x%04x: %04x\n", (unsigned) addr, (unsigned) *addr);
    return 0;
}

static int writem32(char const* addr_s, char const* val_s) {

    char *c;
    uint32_t *addr = (uint32_t *) strtoul(addr_s, &c, 16);

    if (*c != '\0') {

        _kernel_outString("write mem addr invalid\n");
        return -1;
    }

    uint32_t addr_val = (uint32_t) strtoul(val_s, &c, 16);

    if (*c != '\0') {

        _kernel_outString("write value invalid\n");
        return -1;
    }

    *addr = addr_val;

    _kernel_outString("memory write\n");
    return 0;
}