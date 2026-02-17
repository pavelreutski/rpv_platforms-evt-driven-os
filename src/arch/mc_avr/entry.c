#include <avr/interrupt.h>

#include "shell.h"
#include "kernel_stdio.h"

int main(void) {

    _kernel_stdio();
    _kernel_outString("Starting RPV (Co) Event Driven OS (avr5 build)...\n");

    sei(); // enable global interrupts  
    _shell_start();

    return 0;
}