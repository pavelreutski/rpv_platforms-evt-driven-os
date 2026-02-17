#include "shell.h"

#include "sys/xdma.h"
#include "sys/xintc.h"
#include "sys/xuart.h"
#include "sys/rtmsure.h"

#include "kernel_fio.h"
#include "kernel_stdio.h"

extern void _mb_icache_enable(void);
extern void _mb_dcache_enable(void);

extern void _mb_exceptions_enable(void);
extern void _mb_interrupts_enable(void);

int main(void) {

    /* system platform initialization */

    _mb_icache_enable();
    _mb_dcache_enable();

    _xdma_start();
    _rtmsure_start();
    _xuartlite_start();

    _xintc_start();

    _mb_exceptions_enable();
    _mb_interrupts_enable();

    /* ------------------------------ */

    _kernel_fio();
    _kernel_stdio();
    
    _kernel_outString("Starting RPV (Co) Event Driven OS (microblaze build)...\n");

    _shell_start();

    return 0;
}