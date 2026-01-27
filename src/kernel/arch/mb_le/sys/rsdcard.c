#include "sys/xdma.h"
#include "sys/xcache.h"
#include "sys/rsdcard.h"

#define RSDCARD_BASE                        (0x44A00000)

#define RSDCARD                             ((rsdcard_t *) RSDCARD_BASE)

#define SDCARD_SECTOR_SIZE                  (512)

enum {

    IO_READ = 0x01,
    IO_WRITE = 0x02
};

union rsdcard_ctrl_u {

    uint32_t reg;

    struct {

        uint32_t op_code  : 3;  /* Bit 0 - 2 : operation code. Read - 0x01, Write - 0x02 */
        uint32_t c_reset  : 1;  /* Bit 3 : controller reset */
        uint32_t s_count  : 16; /* Bit 4 - 19: operation count - operation sectors count */
        uint32_t reserved : 12; /* Bit 20 - 31: reserved bits */
    };
};

union rsdcard_status_u {

    uint32_t reg;

    struct {

        uint32_t ioc      : 1;  /* Bit 0 : I/O completed flag */
        uint32_t ioe      : 1;  /* Bit 1 : I/O error flag */
        uint32_t idle     : 1;  /* Bit 2 : I/O idle flag */
        uint32_t err_code : 20; /* Bit 3 - 18 : I/O error code when ioe is set */
        uint32_t reserved : 9;  /* Bit 19 - 31 : reserved bits */ 
    };
};

struct rsdcard_s {

    volatile uint32_t addr;             /* 0x00 : address register. sector index */

    volatile union rsdcard_ctrl_u cr;   /* 0x04 : control register */
    volatile union rsdcard_status_u sr; /* 0x08 : status register */

    volatile uint32_t dr;               /* 0x0C : data register */
};

typedef struct rsdcard_s rsdcard_t;

typedef union rsdcard_ctrl_u rsdcard_ctrl_t;
typedef union rsdcard_status_u rsdcard_status_t;

bool _rsdcard_read(volatile void *mem, size_t sector, const size_t count) {

    if (!(RSDCARD -> sr.idle)) {
        return false;
    }

    const uintptr_t m_addr = (const uintptr_t) mem;
    const size_t m_len = (count * SDCARD_SECTOR_SIZE);

    if (!_xdma_s2mm_simple(m_addr, m_len)) {
        return false;
    }

    volatile rsdcard_ctrl_t ctrl_reg = { 0 };

    ctrl_reg.s_count = count;
    ctrl_reg.op_code = IO_READ;

    (RSDCARD -> addr) = (uint32_t) sector;
    (RSDCARD -> cr).reg = ctrl_reg.reg;

    /* Wait for I/O start */
    while ((RSDCARD -> sr.idle)) { }
    
    /* Wait for I/O completed */
    while (!(RSDCARD -> sr.ioc)) { }

    bool io_ok = !(RSDCARD -> sr.ioe);

    if (io_ok) {
        _xdcache_invalidate(m_addr, m_len);
    }

    return io_ok;
}

bool _rsdcard_write(volatile void const* mem, size_t sector, const size_t count) {

    if (!(RSDCARD -> sr.idle)) {
        return false;
    }

    const uintptr_t m_addr = (const uintptr_t) mem;
    const size_t m_len = (count * SDCARD_SECTOR_SIZE);

    _xdcache_flush(m_addr, m_len);

    if (!_xdma_mm2s_simple(m_addr, m_len)) {
        return false;
    }

    volatile rsdcard_ctrl_t ctrl_reg = { 0 };

    ctrl_reg.s_count = count;
    ctrl_reg.op_code = IO_WRITE;

    (RSDCARD -> addr) = (uint32_t) sector;
    (RSDCARD -> cr).reg = ctrl_reg.reg;

    /* Wait for I/O start */
    while ((RSDCARD -> sr.idle)) { }
    
    /* Wait for I/O completed */
    while (!(RSDCARD ->sr.ioc)) { }

    return !(RSDCARD ->sr.ioe);
}