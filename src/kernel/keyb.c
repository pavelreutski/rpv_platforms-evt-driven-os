#include <stdint.h>

#include "print.h"

/// @brief kernel keyboard IRQ handler (ISR)
/// @param scan_code a next received scan code 
void __attribute__((sysv_abi)) _kernel_keybIRQ(uint8_t scan_code) {

    const char* hex = "0123456789ABCDEF";

    print_char(hex[(scan_code >> 4) & 0x0F]);
    print_char(hex[scan_code & 0x0F]);
    
    print_char(' ');
}