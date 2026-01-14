#include "console.h"
#include "private/command.h"

static uint8_t onCls_display(char const* data, const int argc, const char **argv);

_KERNEL_SHELL_COMMAND(cls, onCls_display);

static uint8_t onCls_display(char const* data, const int argc, const char **argv) {

	(void) data;
	(void) argc;
    (void) argv;

	con_clear();
	return EXEC_BUILT_IN;
}