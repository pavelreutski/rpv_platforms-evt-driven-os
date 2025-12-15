#include "shell.h"
#include "kernel_stdio.h"

/// @brief kernel entry point
/// @param none
void __attribute__((sysv_abi)) _cstart_entry(void) {
    
    _kernel_stdio();
    _kernel_outString("Starting RPV (Co) Event Driven OS...\n");

    _shell_start();
}