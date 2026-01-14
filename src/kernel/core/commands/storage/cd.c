#include "kernel_fsh.h"
#include "private/command.h"

static uint8_t onDisk_chDir(char const* data, const int argc, const char **argv);

_KERNEL_SHELL_COMMAND(cd, onDisk_chDir);

static uint8_t onDisk_chDir(char const* data, const int argc, const char **argv) {

    (void) argc;
    (void) argv;

	_kernel_cd(data);
	return EXEC_BUILT_IN;
}
