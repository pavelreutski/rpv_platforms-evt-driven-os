#include "shell.h"

#include "kernel_stdio.h"

int main(void) {

    _kernel_stdio();
    _kernel_outString("Starting RPV (Co) Event Driven OS (microblaze build)...\n");

    _shell_start();

    return 0;
}