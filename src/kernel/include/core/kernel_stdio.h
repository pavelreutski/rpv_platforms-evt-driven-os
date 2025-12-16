#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "events.h"

void _kernel_stdio(void);

void _kernel_outLn(void);
void _kernel_outTab(void);

void _kernel_outInt(const int num);
void _kernel_outFloat(const float num);
void _kernel_outByte(const uint8_t byte);

void _kernel_outChar(const char code);
void _kernel_outString(const char* str);
void _kernel_outStringFormat(const char *fmt, ...);

void _kernel_gotoXY(uint16_t x, uint16_t y);

void _kernel_getKey(console_key_t *key);
bool _kernel_tryGetKey(console_key_t *key);