#include <stdbool.h>

#include "print.h"

void _kernel_main(void) {
    
    print_clear();
    print_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);

    print_string("Welcome to our 64-bit kernel !");

    while (true) { }
}