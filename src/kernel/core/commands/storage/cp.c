#include "command.h"
#include "kernel_exec.h"

#include "utils/cp.h"

static uint8_t onCopy_exec(char const* input, const int argc, const char **argv);

_SHELL_COMMAND(cp, onCopy_exec);
_SHELL_COMMAND(copy, onCopy_exec);

static uint8_t onCopy_exec(char const* input, const int argc, const char **argv) {

    (void) input;
    
	_kernel_exec_f(cp_m, argc, argv);
	return EXEC_EXTERNAL;
}