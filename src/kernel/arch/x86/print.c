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

static volatile cursor_t cursor = { 0, 0 };
static volatile uint8_t char_color =  CGA_COLOR(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);

static volatile cga_char_t *const cga_buffer = (volatile cga_char_t *const)(uintptr_t) CGA_BUFFER_ADDR;

static void print_tab(void);
static void print_del(void);
static void print_newLine(void);

static void clear_row(size_t row);

void print_getxy(cursor_t *c) {

    c -> x = cursor.x;
    c -> y = cursor.y;
}

void print_setxy(cursor_t const* c) {

    if (((c -> x) >= CGA_NUM_COLS) || ((c -> y) >= CGA_NUM_ROWS)) return;
    
    cursor.x = c -> x;
    cursor.y = c -> y;
}

void print_clear(void) {

    cursor.x = 
        cursor.y = 0;

    for (size_t i = 0; i < CGA_NUM_ROWS; i++) {
        clear_row(i);        
    }
}

void print_color(uint8_t fore_color, uint8_t back_color) {
    char_color = CGA_COLOR(fore_color, back_color);
}

void print_char(const char c) {

    switch (c) {

        case '\t': { // TAB
            print_tab(); } break;

        case '\r': { // Cariage return
            cursor.x = 0;
        } break;
            
        case '\n': { // New line
            print_newLine(); } break;

        case '\b':
        case '\x7F': { // DEL
            print_del(); } break;
        
        default: {

            if (cursor.x >= CGA_NUM_COLS) {
                print_newLine();
            }

            cga_buffer[cursor.x + cursor.y * CGA_NUM_COLS] = 
                (cga_char_t) { code : (uint8_t) c, color : char_color };

            cursor.x++; 
        } break;
    }
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

static void print_del(void) {

    if ((cursor.x == 0) && (cursor.y == 0)) return;

    if (cursor.x > 0) {        
        cursor.x--;
    } else if (cursor.y > 0) {

        cursor.y--;
        cursor.x = (CGA_NUM_COLS - 1);
    }

    cga_char_t empty = {
        code: 0,
        color: char_color  
    };

    cga_buffer[cursor.x + cursor.y * CGA_NUM_COLS] = empty;
}

static void print_tab(void) {

    for (size_t i = 0; i < 4; i++) {
        print_char(' ');
    }
}

static void print_newLine(void) {

    cursor.x = 0;

    if (cursor.y < (CGA_NUM_ROWS - 1)) {

        cursor.y++;
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