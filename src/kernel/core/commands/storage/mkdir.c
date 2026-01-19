#include "command.h"
#include "kernel_fsh.h"

static uint8_t onDisk_mkDir(char const* input, const int argc, const char **argv);

_SHELL_COMMAND(mkdir, onDisk_mkDir);

static uint8_t onDisk_mkDir(char const* input, const int argc, const char **argv) {

	(void) argc;
    (void) argv;

	_kernel_mkdir(input);
	return EXEC_BUILT_IN;
}