#include "command.h"
#include "kernel_exec.h"

#include "utils/type.h"

static uint8_t onType_exec(char const* input, const int argc, const char **argv);

_SHELL_COMMAND(type, onType_exec);

static uint8_t onType_exec(char const* input, const int argc, const char **argv) {

    (void) input;

	_kernel_exec_f(type_m, argc, argv);
	return EXEC_EXTERNAL;
}