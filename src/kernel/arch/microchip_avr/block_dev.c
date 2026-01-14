#include "disk.h"

void disk_io(void) { }

size_t get_disks(void) { return 0; }

void get_disk(size_t did, disk_t *disk) { }
void get_diskInfo(size_t did, char *dsk_info, size_t dsk_infoLen) { }

uint8_t flush_disk(size_t did) { return 0; }

uint8_t read_disk(void *buffer, size_t did, size_t blck, size_t blocks) { return 0; }
uint8_t write_disk(void const* buffer, size_t did, size_t blck, size_t blocks) { return 0; } 