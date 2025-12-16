#pragma once

#include <stdint.h>
#include <stddef.h>

enum {

    PRINT_COLOR_BLACK,
    PRINT_COLOR_BLUE,
    PRINT_COLOR_GREEN,
    PRINT_COLOR_CYAN,
    PRINT_COLOR_RED,
    PRINT_COLOR_MAGENTA,
    PRINT_COLOR_BROWN,
    PRINT_COLOR_LIGHT_GRAY,
    PRINT_COLOR_DARK_GRAY,
    PRINT_COLOR_LIGHT_BLUE,
    PRINT_COLOR_LIGHT_GREEN,
    PRINT_COLOR_LIGHT_CYAN,
    PRINT_COLOR_LIGHT_RED,
    PRINT_COLOR_PINK,
    PRINT_COLOR_YELLOW,
    PRINT_COLOR_WHITE
};

typedef struct {

    size_t x;
    size_t y;

} cursor_t;

void print_clear(void);
void print_color(uint8_t fore_color, uint8_t back_color);

void print_char(const char c);
void print_string(const char *s);

void print_getxy(cursor_t *cur);
void print_setxy(cursor_t const* cur);