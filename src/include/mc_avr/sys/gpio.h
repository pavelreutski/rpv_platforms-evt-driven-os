#pragma once

#include <stdint.h>
#include <stdbool.h>

bool _gpiob_read(uint8_t pin);
void _gpiob_write(uint8_t pin, bool val);

bool _gpioc_read(uint8_t pin);
void _gpioc_write(uint8_t pin, bool val);

bool _gpiod_read(uint8_t pin);
void _gpiod_write(uint8_t pin, bool val);