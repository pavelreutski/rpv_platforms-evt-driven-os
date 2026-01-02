#include <string.h>
#include <stdbool.h>

#include "ff.h"
#include "disk.h"
#include "diskio.h"

static bool check_diskReady(BYTE pdrv);
static DRESULT from_diskioStatus(uint8_t dsk_status);

DSTATUS disk_initialize(BYTE pdrv) {
    return disk_status(pdrv);
}

DSTATUS disk_status(BYTE pdrv) {
    return check_diskReady(pdrv) ? 0: STA_NODISK;
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count) {

    if (!check_diskReady(pdrv)) {
        return RES_NOTRDY;
    }

    uint8_t r_stat = read_disk(buff, pdrv, sector, count);

    return from_diskioStatus(r_stat);
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count) {

    if (!check_diskReady(pdrv)) {
        return RES_NOTRDY;
    }

    uint8_t w_stat = write_disk(buff, pdrv, sector, count);

    return from_diskioStatus(w_stat);
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {    

    disk_t disk;
    get_disk(pdrv, &disk);

    if (disk.letter == '\0') {
        return RES_NOTRDY;
    }

    switch (cmd) {

        case GET_SECTOR_COUNT: {

            size_t blocks = (disk.volume / disk.block_size);
            memcpy(buff, (void *) &blocks, sizeof(LBA_t));
        } break;

        case GET_SECTOR_SIZE: {

            size_t block_size = disk.block_size;
            memcpy(buff, (void *) &block_size, sizeof(WORD)); 
        } break;

        case CTRL_SYNC: {

            if (flush_disk(disk.did) != DISK_IO_OK) {
                return RES_ERROR;
            }

        } break;
        
        default:
            return RES_PARERR;
        }

    return RES_OK;
}

static bool check_diskReady(BYTE pdrv) {
    
    disk_t dsk;
    get_disk(pdrv, &dsk);
    return ((dsk.letter) != '\0');
}

static DRESULT from_diskioStatus(uint8_t dsk_status) {

    switch (dsk_status) {

        case DISK_IO_OK:
            return RES_OK;

        case DISK_INVALID:
            return RES_PARERR;

        case DISK_NOT_READY:
            return RES_NOTRDY;
            
        case DISK_WRITE_PROTECT:
            return RES_WRPRT;
        
        default:
            break;
    }

    return RES_ERROR;
}