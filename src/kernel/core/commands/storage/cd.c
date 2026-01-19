#include "command.h"
#include "kernel_fsh.h"

static uint8_t onDisk_chDir(char const* input, const int argc, const char **argv);

_SHELL_COMMAND(cd, onDisk_chDir);

static uint8_t onDisk_chDir(char const* input, const int argc, const char **argv) {

    (void) argc;
    (void) argv;

	_kernel_cd(input);
	return EXEC_BUILT_IN;
}
