#include <stdbool.h>

#include "print.h"

/// @brief kernel entry point
/// @param none
void __attribute__((sysv_abi)) _cstart_entry(void) {
    
    print_clear();
    print_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);

    print_string("Welcome to our 64-bit kernel !");

    while (true) { }
}