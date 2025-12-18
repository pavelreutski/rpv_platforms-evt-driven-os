#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

void con_ln(void);
void con_tab(void);

void con_char(const char code);
void con_string(const char* str);

void con_xy(size_t cx, size_t cy);

void set_con(void);