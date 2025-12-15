#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {

	CON_NONE,
	CON_UART,
	CON_VGA

} console_kinds_t;

void out_Ln(void);
void out_TAB(void);

void out_Int(const int num);
void out_Float(const float num);
void out_Byte(const uint8_t byte);

void out_Char(const char code);
void out_String(const char* str);

void out_XY(size_t x, size_t y);

bool set_console(console_kinds_t kind);