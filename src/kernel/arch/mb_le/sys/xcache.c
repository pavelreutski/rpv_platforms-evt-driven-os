#include <stdint.h>

#include "sys/xcache.h"

/* DCache configuration:
 *      chache lines: 4
 *      policy: write-through
 * 
 * Note: If write-back is going to be used different cache manipulation strategies needs to be used.
 *       Below extern calls are aligned to the write-through policy only */

extern void _mb_l2_cache_flush_range(uintptr_t addr, size_t l);

extern void _mb_l2_cache_invalidate_range(uintptr_t addr, size_t l);
extern void _mb_l1_cache_invalidate_range(uintptr_t addr, size_t l);

void _xdcache_flush(volatile const uintptr_t mem, const size_t s) {

    _mb_l2_cache_flush_range(mem, s);
    _mb_l1_cache_invalidate_range(mem, s);
}

void _xdcache_invalidate(volatile const uintptr_t mem, const size_t s) {

    _mb_l2_cache_invalidate_range(mem, s);
    _mb_l1_cache_invalidate_range(mem, s);
}