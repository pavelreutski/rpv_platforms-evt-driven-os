#include "monitor.h"
#include "console.h"
#include "command.h"

#include "kernel_jrnl.h"
#include "kernel_stdio.h"

static uint8_t onSysJournal_display(char const* data, const int argc, const char **argv);

_SHELL_COMMAND(sysj, onSysJournal_display);

static uint8_t onSysJournal_display(char const* data, const int argc, const char **argv) {

	(void) data;
	(void) argc;
    (void) argv;

	char entry[256];
	size_t j_entries = 0;

	con_clear();

	while (_kernel_jnxtentry(entry, sizeof(entry))) {

		j_entries++;
		text_monitor(entry);		
	}

	if (j_entries == 0) {
		_kernel_outString("system journal has no new entries\n");
	}

	return EXEC_BUILT_IN;
}