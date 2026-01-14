#include "version.h"

#include "kernel_stdio.h"
#include "private/command.h"

static uint8_t onVer_display(char const* data, const int argc, const char **argv);

_KERNEL_SHELL_COMMAND(ver, onVer_display);

static uint8_t onVer_display(char const* data, const int argc, const char **argv) {

	(void) data;
	(void) argc;
    (void) argv;

	_kernel_outStringFormat(
		"Event Driven OS [Version %d.%d.%d-%s]\n", 
			MAJOR, MINOR, PATCH, REVISION);

	return EXEC_BUILT_IN;
}