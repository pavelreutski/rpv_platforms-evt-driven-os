#include <stdint.h>

#include "kernel_stdio.h"

void _xsoft_except(uint32_t ear, uint32_t esr) {
    _kernel_outStringFormat("software exception occured: ear=%04x, esr=%04x\n", ear, esr);
}

void _xhard_except(uint32_t ear, uint32_t esr) {
    _kernel_outStringFormat("hardware exception occured: ear=%04x, esr=%04x\n", ear, esr);
}