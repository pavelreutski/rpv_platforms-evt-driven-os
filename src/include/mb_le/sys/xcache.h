#pragma once

#include <stdint.h>
#include <stddef.h>

void _xdcache_flush(const uintptr_t mem, const size_t s);
void _xdcache_invalidate(const uintptr_t mem, const size_t s);