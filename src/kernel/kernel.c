#include <stdbool.h>

#include "print.h"

void kernel_main(void) {
    
    do {

        print_clear();
        print_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);

        print_string("Welcome to our 64-bit kernel !");

        for(;;) { }

    } while (true);
}