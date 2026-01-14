#include "kernel_exec.h"

#include "utils/type.h"
#include "private/command.h"

static uint8_t onType_exec(char const* data, const int argc, const char **argv);

_KERNEL_SHELL_COMMAND(type, onType_exec);

static uint8_t onType_exec(char const* data, const int argc, const char **argv) {

    (void) data;

	_kernel_exec_f(type_m, argc, argv);
	return EXEC_EXTERNAL;
}