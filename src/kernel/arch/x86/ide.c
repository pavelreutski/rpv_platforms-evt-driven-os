#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "diskio.h"
#include "kernel_stdio.h"

#define IDE_DEV_BUSES               (2)
#define IDE_BUS_DEVICES             (2)

#define IDE_DEV_LETTERS             (4)
#define IDE_DEV_ID_WORDS            (256)

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

    uint16_t identity_block[IDE_DEV_ID_WORDS];

} ide_dev_t;

static ide_dev_t ide_dev[IDE_DEV_LETTERS];

extern uint8_t __attribute__((sysv_abi)) _arch_ide_device_detect(uint8_t ide_bus, uint8_t ide_device, uint16_t *identity);

extern uint8_t __attribute__((sysv_abi)) _arch_ide_device_read(uint8_t ide_bus, uint8_t ide_device, void *buffer, size_t lba_sector, size_t sectors);
extern uint8_t __attribute__((sysv_abi)) _arch_ide_device_write(uint8_t ide_bus, uint8_t ide_device, void const* buffer, size_t lba_sector, size_t sectors);

void disk_io(void) {

    uint8_t dev_letter = 0;
    char letters[] = { 'C', 'D', 'E', 'F' };

    ide_dev_t *dev = ide_dev;

    for (size_t ide_bus = 0; ide_bus < IDE_DEV_BUSES; ide_bus++) {
        for (size_t ide_bus_dev = 0; ide_bus_dev < IDE_BUS_DEVICES; ide_bus_dev++) {

            dev -> ide_bus = ide_bus;
            dev -> bus_dev = ide_bus_dev;

            dev -> letter = letters[dev_letter];

            dev -> ata_detect = 
                _arch_ide_device_detect(
                    ide_bus, ide_bus_dev, dev -> identity_block);            

            dev_letter++;
            dev = &ide_dev[dev_letter];            
        }
    }
}

size_t get_disks(void) {
    return IDE_DEV_LETTERS;
}

void get_disk(size_t did, disk_t *disk) {

    if ((disk == NULL) || (did >= IDE_DEV_LETTERS)) return;

    disk -> did = did;
    disk -> volume = 0;

    disk -> letter = ide_dev[did].letter;
}

void get_diskInfo(size_t did, char *dsk_info, size_t dsk_infoLen) {

    const size_t max_dskInfo = 255;

    if ((did >= IDE_DEV_LETTERS) || 
            (dsk_info == NULL) || (dsk_infoLen > max_dskInfo)) return;

    ide_dev_t *dev = &ide_dev[did];

    // IDE: bus 0/1, device 0/1, (not detected)/ata/atapi

    _kernel_stringFormat(
        dsk_info, "IDE: bus %d, device %d, ", dev -> ide_bus, dev -> bus_dev);

    switch (dev -> ata_detect) {
    
    case IDE_ATA_DEVICE: { strcat(dsk_info, "ata");  } break;
    case IDE_ATAPI_DEVICE: { strcat(dsk_info, "atapi"); } break;
    case IDE_NO_DEVICE:  { strcat(dsk_info, "not detected"); } break;
    
    default: { strcat(dsk_info, "unknown"); } break;

    }
}

uint8_t read_disk(void *buffer, size_t did, size_t blck, size_t blocks) {

    if (did > IDE_DEV_LETTERS) return DISK_INVALID;

    ide_dev_t *dev = &ide_dev[did];

    if (dev -> ata_detect == IDE_NO_DEVICE) {
        return DISK_NOT_READY;
    }

    uint8_t is_readOk = 
        _arch_ide_device_read(dev -> ide_bus, dev -> bus_dev, buffer, blck, blocks);

    return is_readOk ? DISK_IO_OK : DISK_IO_ERR;
}

uint8_t write_disk(void const* buffer, size_t did, size_t blck, size_t blocks) {
    
    (void) did;
    (void) blck;
    (void) buffer;
    (void) blocks;

    return DISK_WRITE_PROTECT;
}
