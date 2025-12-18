#include "print.h"
#include "console.h"

extern void __attribute__((sysv_abi)) _arch_hide_cursor(void);
extern void __attribute__((sysv_abi)) _arch_show_cursor(void);

extern void __attribute__((sysv_abi)) _arch_set_cursor_pos(size_t row, size_t col);

static void __attribute__((noinline)) move_cursor(void);

void con_ln(void) {

    print_char('\n');
    move_cursor();
}

void con_tab(void) {

    print_char('\t');
    move_cursor();
}

void con_char(const char code) {

    print_char(code);
    move_cursor();
}

void con_string(const char* str) {

    print_string(str);
    move_cursor();
}

void con_xy(size_t cx, size_t cy) {

    cursor_t c = { x: cx, y: cy };

    print_setxy(&c);
    print_getxy(&c);

    _arch_set_cursor_pos(c.y, c.x);
}

void set_con(void) {

    print_clear();
    print_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);

    move_cursor();
    _arch_show_cursor();
}

static void move_cursor(void) {

    cursor_t c;
    print_getxy(&c);

    _arch_set_cursor_pos(c.y, c.x);
}