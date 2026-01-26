#include <stdint.h>

#include "sys/xcache.h"

extern void _mb_l2_cache_invalidate_range(uintptr_t addr, size_t l);
extern void _mb_l1_cache_invalidate_range(uintptr_t addr, size_t l);

void _xdcache_invalidate(void const* mem, size_t s) {

    _mb_l2_cache_invalidate_range((uintptr_t) mem, s);
    _mb_l1_cache_invalidate_range((uintptr_t) mem, s);
}