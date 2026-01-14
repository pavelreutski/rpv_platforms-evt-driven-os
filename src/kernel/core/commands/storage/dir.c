#include "kernel_fsh.h"
#include "private/command.h"

static uint8_t onDiskDir_display(char const* data, const int argc, const char **argv);

_KERNEL_SHELL_COMMAND(dir, onDiskDir_display);

static uint8_t onDiskDir_display(char const* data, const int argc, const char **argv) {

	(void) data;
	
	_kernel_ls(argc, (const char **) argv);
	return EXEC_BUILT_IN;
}
