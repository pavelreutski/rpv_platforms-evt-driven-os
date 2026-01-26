#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

bool rsdcard_read(void *mem, size_t sector, size_t count);
bool rsdcard_write(void const* mem, size_t sector, size_t count);