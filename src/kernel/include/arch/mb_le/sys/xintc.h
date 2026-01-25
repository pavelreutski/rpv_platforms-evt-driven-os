#pragma once

#include <stdint.h>

void _xintc_start(void);

void _xintc_disableIRQ(const uint8_t irq);
void _xintc_enableIRQ(const uint8_t irq, void (*const isr)(void));