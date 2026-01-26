#include <string.h>

#include "disk.h"
#include "kernel_stdio.h"

#include "sys/xcache.h"
#include "sys/rsdcard.h"

#define MAX_DISKS                   (1)
#define MAX_BLOCK_SIZE              (512)

#define DSK_IO_BUFFER               (0x80000000)

static volatile void *io_buffer     = (void *) DSK_IO_BUFFER;

void disk_io(void) {
}

size_t get_disks(void) {
    return MAX_DISKS;
}

void get_disk(size_t did, disk_t *disk) {
    
    if ((disk == NULL) || (did >= MAX_DISKS)) {
        return;
    }

    disk -> did = did;
    disk -> num_blocks = 0;
    disk -> block_size = MAX_BLOCK_SIZE;

    disk -> volume = 0;
    disk -> letter = 'C';
}

void get_diskInfo(size_t did, char *dsk_info, size_t dsk_infoLen) {

    if ((dsk_info == NULL) || (did >= MAX_DISKS) || (dsk_infoLen > 256)) {
        return;
    }

    int dsk_vol = 0;
    _kernel_stringFormat(dsk_info, "sdcard#%d, volume %d", did, dsk_vol);
}

uint8_t flush_disk(size_t did) {
    return did == (MAX_DISKS - 1) ? DISK_IO_OK : DISK_INVALID;
}

uint8_t read_disk(void *buffer, size_t did, size_t blck, size_t blocks) {

    if ((buffer == NULL) || (did >= MAX_DISKS)) {
        return DISK_INVALID;
    }

    if (!rsdcard_read((void *) io_buffer, blck, blocks)) {
        return DISK_IO_ERR;
    }

    size_t blck_s = (blocks * MAX_BLOCK_SIZE);

    _xdcache_invalidate((void *) io_buffer, blck_s);
    memcpy(buffer, (const void *) io_buffer, blck_s);

    return DISK_IO_OK;
}

uint8_t write_disk(void const* buffer, size_t did, size_t blck, size_t blocks) {

    if ((buffer == NULL) || (did >= MAX_DISKS)) {
        return DISK_INVALID;
    }

    return rsdcard_write(buffer, blck, blocks) ? DISK_IO_OK : DISK_IO_ERR;
}