#include <stddef.h>

#include "print.h"

#define CGA_NUM_COLS                        (80)
#define CGA_NUM_ROWS                        (25)

#define CGA_BUFFER_ADDR                     (0xb8000)

#define CGA_COLOR(fore_c, back_c)           (fore_c | (back_c << 4))

struct __attribute__((packed, aligned(1))) cga_char {

    uint8_t code;
    uint8_t color;
};

typedef struct cga_char cga_char_t;

static size_t cursor_col = 0;
static size_t cursor_row = 0;

static uint8_t char_color =  CGA_COLOR(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);

static volatile cga_char_t *const cga_buffer = (volatile cga_char_t *const)(uintptr_t) CGA_BUFFER_ADDR;

static void print_newLine(void);
static void clear_row(size_t row);

void print_clear(void) {

    cursor_col = 0;
    cursor_row = 0;

    for (size_t i = 0; i < CGA_NUM_ROWS; i++) {
        clear_row(i);        
    }
}

void print_color(uint8_t fore_color, uint8_t back_color) {
    char_color = CGA_COLOR(fore_color, back_color);
}

void print_char(const char c) {

    if (c == '\n') {

        print_newLine();
        return;
    }

    if (cursor_col > CGA_NUM_COLS) {
        print_newLine();
    }

    cga_buffer[cursor_col + cursor_row * CGA_NUM_ROWS] = 
        (cga_char_t) { code : (uint8_t) c, color : char_color };

    cursor_col++;
}

void print_string(const char *s) {

    for(size_t i = 0; s[i] != 0; i++) {

        char c = s[i];
        print_char(c);
    }
}

static void clear_row(size_t row) {

    cga_char_t empty = {
        code: 0,
        color: char_color  
    };

    for (size_t i = 0; i < CGA_NUM_COLS; i++) {        
        cga_buffer[i + row * CGA_NUM_COLS] = empty;        
    }
}

static void print_newLine(void) {

    cursor_col = 0;

    if (cursor_row < CGA_NUM_ROWS) {

        cursor_row++;
        return;
    }

    for (size_t row = 1; row < CGA_NUM_ROWS; row++) {
        for (size_t col = 0; col < CGA_NUM_COLS; col++) {

            cga_char_t cga_c = cga_buffer[col + row * CGA_NUM_COLS];
            cga_buffer[col + (row - 1) * CGA_NUM_COLS] = cga_c;
        }
    }
    
    clear_row(CGA_NUM_ROWS - 1);
}