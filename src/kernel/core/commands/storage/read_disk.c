#include <stdlib.h>

#include "disk.h"
#include "monitor.h"

#include "kernel_stdio.h"
#include "private/command.h"

static uint8_t onDisk_blckRead(char const* data, const int argc, const char **argv);

_KERNEL_SHELL_COMMAND(dsk_block, onDisk_blckRead);

static uint8_t onDisk_blckRead(char const* data, const int argc, const char **argv) {

    (void) data;

	if (argc != 2) {

		_kernel_outString("Invalid parameters. Disk id and block numer (zero based) is required\n");
		return EXEC_BUILT_IN;
	}
	
	uint8_t blck_data[512];

	size_t did = atol(argv[0]);
	size_t blck = atol(argv[1]);

	uint8_t disk_io = read_disk(blck_data, did, blck, 1);

	switch (disk_io) {

		case DISK_INVALID: { _kernel_outString("drive invalid !\n"); } break;
		case DISK_NOT_READY: { _kernel_outString("drive not ready !\n"); } break;
 		case DISK_IO_ERR: { _kernel_outString("drive read block failed !\n"); } break;
	}

	if (disk_io != DISK_IO_OK) {
		return EXEC_BUILT_IN;
	}

	hex_monitor(blck_data, sizeof(blck_data));

	return EXEC_BUILT_IN;
}