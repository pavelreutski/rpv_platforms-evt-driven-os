#include <stddef.h>

#include "uart.h"
#include "console.h"

size_t MAX_X = 0;
size_t MAX_Y = 0;

void con_clear(void) { }

void set_con(void) {
    _uart_start();
}

void con_ln(void) {
    con_char('\n');
}

void con_tab(void) {
    con_char('\t');
}

void con_char(const char code) {

    if (code == '\x7f') {

        con_char('\b');
        con_char(' ');
        con_char('\b');

        return;
    }

    _uart_write(code);
}

void con_string(const char* str) {
    
    for (char *s = (char *) str; 
                *s != '\0'; s++) {
                    
        con_char((*s));
    }
}

void con_xy(size_t cx, size_t cy) {

    (void) cx;
    (void) cy;
}

size_t con_getx(void) { return 0; }
size_t con_gety(void) { return 0; }