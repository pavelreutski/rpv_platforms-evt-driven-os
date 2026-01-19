#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "disk.h"

#include "kernel_jrnl.h"
#include "kernel_stdio.h"

#define IDE_DEV_BUSES               (2)
#define IDE_BUS_DEVICES             (2)

#define IDE_DEV_LETTERS             (4)
#define IDE_NO_LETTER               ('\0')

/* LBA48 only */

#define IDE_CAP_LBA_LO              (60)
#define IDE_CAP_LBA_HI              (61)

#define IDE_DEV_ID_WORDS            (256)

#define IDE_ATA_SECTOR_BYTES        (512)
#define IDE_ATAPI_SECTOR_BYTES      (2048)

enum {
    IDE_NO_DEVICE,
    IDE_ATA_DEVICE,
    IDE_ATAPI_DEVICE
};

enum {
    IDE_PRIMARY_BUS,
    IDE_SECONDARY_BUS
};

enum {
    IDE_MASTER,
    IDE_SLAVE
};

typedef struct {

    char letter;

    uint8_t ide_bus;
    uint8_t bus_dev;

    uint8_t ata_detect;

    size_t capacity;

    uint16_t identity_block[IDE_DEV_ID_WORDS];

} ide_dev_t;

static ide_dev_t ide_dev[IDE_DEV_LETTERS];

extern uint8_t __attribute__((sysv_abi)) _arch_ide_device_detect(uint8_t ide_bus, uint8_t ide_device, uint16_t *identity);

extern uint8_t __attribute__((sysv_abi)) _arch_ide_device_flush(uint8_t ide_bus, uint8_t ide_device);
extern uint8_t __attribute__((sysv_abi)) _arch_ide_device_read(uint8_t ide_bus, uint8_t ide_device, void *buffer, size_t lba_sector, size_t sectors);
extern uint8_t __attribute__((sysv_abi)) _arch_ide_device_write(uint8_t ide_bus, uint8_t ide_device, void const* buffer, size_t lba_sector, size_t sectors);

void disk_io(void) {

    size_t dev_letter = 0;
    char letters[] = { 'C', 'D', 'E', 'F' };

    ide_dev_t *dev = ide_dev;

    for (size_t ide_bus = 0; ide_bus < IDE_DEV_BUSES; ide_bus++) {
        for (size_t ide_bus_dev = 0; ide_bus_dev < IDE_BUS_DEVICES; ide_bus_dev++, dev++) {

            dev -> ide_bus = ide_bus;
            dev -> bus_dev = ide_bus_dev;

            dev -> capacity = 0;
            dev -> letter = IDE_NO_LETTER;

            memset((dev -> identity_block), 0, sizeof(dev->identity_block));

            dev -> ata_detect = 
                _arch_ide_device_detect(
                    ide_bus, ide_bus_dev, dev -> identity_block);
            
            if (dev -> ata_detect != IDE_NO_DEVICE) {                

                size_t cap_sectors = (dev -> identity_block[IDE_CAP_LBA_HI] << 16) | 
                                        (dev -> identity_block[IDE_CAP_LBA_LO]);

                dev -> letter = letters[dev_letter];
                dev -> capacity = (cap_sectors * IDE_ATA_SECTOR_BYTES);

                dev_letter++;
            }           
        }
    }
}

size_t get_disks(void) {
    return IDE_DEV_LETTERS;
}

void get_disk(size_t did, disk_t *disk) {

    if ((disk == NULL) || (did >= IDE_DEV_LETTERS)) return;

    ide_dev_t *dev = &ide_dev[did];

    disk -> did = did;

    disk -> letter = dev -> letter;
    disk -> volume = dev -> capacity;

    disk -> block_size = 0;

    switch (dev -> ata_detect) {

        case IDE_ATA_DEVICE:
            disk -> block_size = IDE_ATA_SECTOR_BYTES;
            break;

        case IDE_ATAPI_DEVICE:
            disk -> block_size = IDE_ATAPI_SECTOR_BYTES;
            break;
    
        default:
            break;
    }
}

void get_diskInfo(size_t did, char *dsk_info, size_t dsk_infoLen) {

    if ((did >= IDE_DEV_LETTERS) || 
            (dsk_info == NULL) || (dsk_infoLen > 256)) return;

    ide_dev_t *dev = &ide_dev[did];

    // IDE#did: bus 0/1, device 0/1, (not detected)/ata/atapi, capacity (when ata/atapi)

    _kernel_stringFormat(
        dsk_info, "IDE#%d: bus %d, device %d, ", did, dev -> ide_bus, dev -> bus_dev);

    switch (dev -> ata_detect) {

        case IDE_ATAPI_DEVICE: { strcat(dsk_info, "atapi"); } break;
        case IDE_NO_DEVICE:  { strcat(dsk_info, "not detected"); } break;

        case IDE_ATA_DEVICE: { 
            _kernel_stringFormat(
                (dsk_info + strlen(dsk_info)), "ata, capacity %d", dev -> capacity); 
        } break;
        
        default: { strcat(dsk_info, "unknown"); } break;
    }
}

uint8_t flush_disk(size_t did) {

    if (did >= IDE_DEV_LETTERS) return DISK_INVALID;

    ide_dev_t *dev = &ide_dev[did];

    if (dev -> ata_detect == IDE_NO_DEVICE) {
        return DISK_NOT_READY;
    }

    uint8_t is_flushOk = _arch_ide_device_flush(dev -> ide_bus, dev -> bus_dev);

    if (!is_flushOk) {
        _kernel_jentry(
            "[kernel] error: device#%d on ide bus#%d flush data failed", (int) dev -> bus_dev, (int) dev -> ide_bus);
    }

    return is_flushOk ? DISK_IO_OK : DISK_IO_ERR;
}

uint8_t read_disk(void *buffer, size_t did, size_t blck, size_t blocks) {

    if (did >= IDE_DEV_LETTERS) return DISK_INVALID;

    ide_dev_t *dev = &ide_dev[did];

    if (dev -> ata_detect == IDE_NO_DEVICE) {
        return DISK_NOT_READY;
    }

    uint8_t is_readOk = 
        _arch_ide_device_read(dev -> ide_bus, dev -> bus_dev, buffer, blck, blocks);

    if (!is_readOk) {
        _kernel_jentry("[kernel] error: read disk sector#%d count#%d failed", (int) blck, (int) blocks);
    }

    return is_readOk ? DISK_IO_OK : DISK_IO_ERR;
}

uint8_t write_disk(void const* buffer, size_t did, size_t blck, size_t blocks) {

    if (did >= IDE_DEV_LETTERS) return DISK_INVALID;

    ide_dev_t *dev = &ide_dev[did];

    if (dev -> ata_detect == IDE_NO_DEVICE) {
        return DISK_NOT_READY;
    }

    uint8_t is_writeOk = 
        _arch_ide_device_write(dev -> ide_bus, dev -> bus_dev, buffer, blck, blocks);

    if (!is_writeOk) {
        _kernel_jentry("[kernel] error: write disk sector#%d count#%d failed", (int) blck, (int) blocks);
    }

    return is_writeOk ? DISK_IO_OK : DISK_IO_ERR;
}
