#include "command.h"
#include "kernel_fsh.h"

static uint8_t onDiskFile_rm(char const* input, const int argc, const char **argv);

_SHELL_COMMAND(rm, onDiskFile_rm);
_SHELL_COMMAND(rmdisk, onDiskFile_rm);

static uint8_t onDiskFile_rm(char const* input, const int argc, const char **argv) {

	(void) argc;
    (void) argv;

	_kernel_rmfile(input);
	return EXEC_BUILT_IN;
}