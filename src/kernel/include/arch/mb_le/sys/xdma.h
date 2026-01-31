#pragma once

#include <stddef.h>

void _xdma_start(void);

volatile void const* _xdma_s2mm_simple(const size_t len);
volatile void const* _xdma_mm2s_simple(void const* mem, const size_t len);

void _xdma_mm2s_sgstop(void);

bool _xdma_mm2s_sgcmpltIRQ(void);

void *_xdma_mm2s_sgcmplt(const size_t len);
volatile void const* _xdma_mm2s_sgcyclic(const int seed, const size_t len, const size_t chunk);