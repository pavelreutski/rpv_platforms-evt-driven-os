#pragma once

#include <stddef.h>

void _xdma_start(void);

volatile void const* _xdma_s2mm_simple(const size_t len);
volatile void const* _xdma_mm2s_simple(void const* mem, const size_t len);

void _xdma2_mm2s_sgstop(void);
void _xdma2_s2mm_sgstop(void);

void _xdma1_mm2s_sgstop(void);

bool _xdma1_mm2s_sgcmpltSignal(void);
bool _xdma1_mm2s_sgbuserrSignal(void);
void *_xdma1_mm2s_sgcmplt(const size_t len);

bool _xdma2_s2mm_sgcmpltSignal(void);
void *_xdma2_s2mm_sgcmplt(void *mem, const size_t mem_len, size_t *const trans_len);

volatile void const* _xdma2_mm2s_sg(void const* mem, const size_t len);
volatile void const* _xdma2_s2mm_sgcyclic(const size_t len, const size_t chunk);

volatile void const* _xdma1_mm2s_sgcyclic(const int seed, const size_t len, const size_t chunk);