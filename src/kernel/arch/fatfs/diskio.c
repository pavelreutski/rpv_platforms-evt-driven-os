#include <string.h>

#include "ff.h"
#include "disk.h"
#include "diskio.h"

DSTATUS disk_initialize(BYTE pdrv) {
    return disk_status(pdrv);
}

DSTATUS disk_status(BYTE pdrv) {

    disk_t disk;
    get_disk(pdrv, &disk);

    return (disk.letter == '\0') ? STA_NODISK : 0;
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count) {

    disk_t disk;
    get_disk(pdrv, &disk);

    if (disk.letter == '\0') {
        return RES_NOTRDY;
    }

    uint8_t r_stat = read_disk(buff, disk.did, sector, count);

    switch (r_stat) {

        case DISK_IO_ERR:
            return RES_ERROR;
            
        case DISK_NOT_READY:
            return RES_NOTRDY;

        case DISK_INVALID:
            return RES_PARERR;
    }

    return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count) {

    (void) pdrv;
    (void) buff;
    (void) sector;
    (void) count;

    return RES_WRPRT;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {

    disk_t disk;
    get_disk(pdrv, &disk);

    if (disk.letter == '\0') {
        return RES_NOTRDY;
    }

    switch (cmd) {

    case GET_SECTOR_SIZE: {

        size_t block_size = disk.block_size;
        memcpy(buff, (void *) &block_size, sizeof(WORD)); } break;
    
    default:
        return RES_PARERR;
    }

    return RES_OK;
}