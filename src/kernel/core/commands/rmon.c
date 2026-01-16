#include <stdlib.h>

#include "monitor.h"
#include "console.h"

#include "kernel_stdio.h"
#include "private/command.h"

static uint8_t onMem_display(char const* data, const int argc, const char **argv);

_KERNEL_SHELL_COMMAND(rmon, onMem_display);

static uint8_t onMem_display(char const* data, const int argc, const char **argv) {

    (void) data;

    if (argc < 2) {

        _kernel_outString("mem addr and size are required\n");
        return EXEC_BUILT_IN;
    }

    size_t m_len = atol(argv[1]);

    char *e_arg;
    size_t m_addr = strtoul(argv[0], &e_arg, 16);

    if (m_len == 0) {

        _kernel_outString("mem size invalid\n");
        return EXEC_BUILT_IN;
    }

    if ((*e_arg) != '\0') {

        _kernel_outString("mem addr invalid\n");
        return EXEC_BUILT_IN;
    }

    con_clear();

    _kernel_outStringFormat("0x%08x %d\n", (unsigned) m_addr, (int) m_len);
    hex_monitor((void *) m_addr, m_len);

    return EXEC_BUILT_IN;
}