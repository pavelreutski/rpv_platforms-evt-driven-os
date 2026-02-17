#include "shell.h"

#include "kernel_fio.h"
#include "kernel_stdio.h"

/// @brief kernel entry point
/// @param none
void _cstart_entry(void) {
    
    _kernel_fio();
    _kernel_stdio();

    _kernel_outString("Starting RPV (Co) Event Driven OS (x86_64 build)...\n");

    _shell_start();
}