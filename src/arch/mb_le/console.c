#include "tty.h"
#include "console.h"

#include "sys/xuart.h"

size_t MAX_X = 0;
size_t MAX_Y = 0;

void con_clear(void) { }

void set_con(void) {
    _xuartlite_start();
}

void con_ln(void) {
    con_char('\n');
}

void con_tab(void) {
    con_char('\t');
}

void con_char(const char code) {
    tty_write(code);
}

void con_string(const char* str) {
    tty_string(str);
}

size_t con_getx(void) {
    return 0;
}

size_t con_gety(void) {
    return 0;
}

void con_xy(size_t cx, size_t cy) {

    (void) cx;
    (void) cy;
}