#pragma once

#include <stddef.h>

void _sec_runtime(void);
void _sec_context(void const* protected_region, const size_t region_size);