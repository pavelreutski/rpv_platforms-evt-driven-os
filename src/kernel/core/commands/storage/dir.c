#include "command.h"
#include "kernel_fsh.h"

static uint8_t onDiskDir_display(char const* input, const int argc, const char **argv);

_SHELL_COMMAND(dir, onDiskDir_display);

static uint8_t onDiskDir_display(char const* input, const int argc, const char **argv) {

	(void) input;
	
	_kernel_ls(argc, (const char **) argv);
	return EXEC_BUILT_IN;
}
