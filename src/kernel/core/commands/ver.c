#include "version.h"
#include "command.h"

#include "kernel_stdio.h"

static uint8_t onVer_display(char const* input, const int argc, const char **argv);

_SHELL_COMMAND(ver, onVer_display);

static uint8_t onVer_display(char const* input, const int argc, const char **argv) {

	(void) input;

	(void) argc;
    (void) argv;

	_kernel_outStringFormat(
		"Event Driven OS [Version %d.%d.%d-%s]\n", 
			MAJOR, MINOR, PATCH, REVISION);

	return EXEC_BUILT_IN;
}