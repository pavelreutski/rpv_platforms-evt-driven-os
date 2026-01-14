#include "kernel_fsh.h"
#include "private/command.h"

static uint8_t onDiskFile_rm(char const* data, const int argc, const char **argv);

_KERNEL_SHELL_COMMAND(rm, onDiskFile_rm);
_KERNEL_SHELL_COMMAND(rmdisk, onDiskFile_rm);

static uint8_t onDiskFile_rm(char const* data, const int argc, const char **argv) {

	(void) argc;
    (void) argv;

	_kernel_rmfile(data);
	return EXEC_BUILT_IN;
}