#include "kernel_fsh.h"
#include "private/command.h"

static uint8_t onDisk_mkDir(char const* data, const int argc, const char **argv);

_KERNEL_SHELL_COMMAND(mkdir, onDisk_mkDir);

static uint8_t onDisk_mkDir(char const* data, const int argc, const char **argv) {

	(void) argc;
    (void) argv;

	_kernel_mkdir(data);
	return EXEC_BUILT_IN;
}