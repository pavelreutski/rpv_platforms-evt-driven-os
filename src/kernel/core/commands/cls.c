#include "console.h"
#include "command.h"

static uint8_t onCls_display(char const* data, const int argc, const char **argv);

_SHELL_COMMAND(cls, onCls_display);

static uint8_t onCls_display(char const* input, const int argc, const char **argv) {

	(void) argc;
    (void) argv;

	(void) input;

	con_clear();
	return EXEC_BUILT_IN;
}