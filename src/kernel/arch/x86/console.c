#include "print.h"
#include "console.h"

extern void __attribute__((sysv_abi)) _arch_hide_cursor(void);
extern void __attribute__((sysv_abi)) _arch_show_cursor(void);

extern void __attribute__((sysv_abi)) _arch_set_cursor_pos(size_t row, size_t col);

static void __attribute__((noinline)) move_cursor(void);

void out_Ln(void) {

    print_char('\n');
    move_cursor();
}

void out_TAB(void) {

    print_char('\t');
    move_cursor();
}

void out_Byte(const uint8_t byte) {
    out_Int(byte);
}

void out_Int(const int num) {

    if (num == 0) {

        print_char('0');
        move_cursor();

        return;
    }

    size_t d;
    bool is_print_started = false;

    if (num < 0) {

        d = ((~(int64_t)num) + 1);
        print_char('-');
    } else {
        d = (size_t) num;
    }

    int n = num;
    int divisor = 1;

    while ((n / divisor >= 10) || (n / divisor) <= -10) {
        divisor *= 10;
    }

    while(divisor) {

        int digit = (d / divisor);

        if ((digit != 0) || is_print_started) {

            is_print_started = true;
            print_char(digit + '0');
        }

        d %= divisor;
        divisor /= 10;
    }

    move_cursor();
}

void out_Float(const float num) {

    (void) num;

    print_char('F'); // todo: implement float to ASCII conversion
    move_cursor();
}

void out_Char(const char code) {

    print_char(code);
    move_cursor();
}

void out_String(const char* str) {

    print_string(str);
    move_cursor();
}

void out_XY(size_t cx, size_t cy) {

    cursor_t c = { x: cx, y: cy };

    print_setxy(&c);
    print_getxy(&c);

    _arch_set_cursor_pos(c.y, c.x);
}

void set_console(void) {

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