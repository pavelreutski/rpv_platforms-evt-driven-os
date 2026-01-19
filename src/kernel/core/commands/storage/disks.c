#include "disk.h"
#include "command.h"

#include "kernel_stdio.h"

static uint8_t onDisk_info(char const* input, const int argc, const char **argv);

_SHELL_COMMAND(disks, onDisk_info);

static uint8_t onDisk_info(char const* input, const int argc, const char **argv) {

    (void) argc;
    (void) argv;

	(void) input;

	size_t n_disks = get_disks();

	for (size_t i = 0; i < n_disks; i++) {

		char dsk_info[255];
		get_diskInfo(i, dsk_info, sizeof(dsk_info));

		_kernel_outStringFormat("%s\n", dsk_info);		
	}

	return EXEC_BUILT_IN;
}