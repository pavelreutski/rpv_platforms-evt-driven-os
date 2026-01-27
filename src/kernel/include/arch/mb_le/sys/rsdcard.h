#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

bool _rsdcard_read(volatile void *mem, size_t sector, const size_t count);
bool _rsdcard_write(volatile void const* mem, size_t sector, const size_t count);