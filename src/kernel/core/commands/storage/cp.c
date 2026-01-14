#include "kernel_exec.h"

#include "utils/cp.h"
#include "private/command.h"

static uint8_t onCopy_exec(char const* data, const int argc, const char **argv);

_KERNEL_SHELL_COMMAND(cp, onCopy_exec);
_KERNEL_SHELL_COMMAND(copy, onCopy_exec);

static uint8_t onCopy_exec(char const* data, const int argc, const char **argv) {

    (void) data;
    
	_kernel_exec_f(cp_m, argc, argv);
	return EXEC_EXTERNAL;
}