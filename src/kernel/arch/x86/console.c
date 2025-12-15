#include "print.h"
#include "console.h"

void out_Ln(void) {
    print_char('\n');
}

void out_TAB(void) {
    print_char('\t');
}

void out_Int(const int num) {
    (void) num;
    print_char('I'); // todo : implement int to ASCII conversion
}

void out_Float(const float num) {
    (void) num;
    print_char('F'); // todo: implement float to ASCII conversion
}

void out_Byte(const uint8_t byte) {
    (void) byte;
    print_char('B'); // todo: implement byte to ASCII conversion
}

void out_Char(const char code) {
    print_char(code);
}

void out_String(const char* str) {
    print_string(str);
}

void out_XY(size_t x, size_t y) {
    print_xy(x, y); 
}

bool set_console(console_kinds_t kind) {

    if (kind != CON_VGA) return false;

    print_clear();
    print_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);

    return true;
}