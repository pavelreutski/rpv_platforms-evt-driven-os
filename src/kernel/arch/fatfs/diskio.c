#include "ff.h"
#include "disk.h"
#include "diskio.h"

DSTATUS disk_initialize(BYTE pdrv) {
    (void) pdrv;
    return 0;
}

DSTATUS disk_status(BYTE pdrv) {
    (void) pdrv;
    return 0;
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count) {

    (void) pdrv;
    (void) buff;
    (void) sector;
    (void) count;

    return RES_ERROR;
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count) {

    (void) pdrv;
    (void) buff;
    (void) sector;
    (void) count;

    return RES_ERROR;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {

    (void) pdrv;
    (void) cmd;
    (void) buff;

    return RES_ERROR;
}