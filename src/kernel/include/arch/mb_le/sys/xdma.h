#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

void _xdma_start(void);

void _xdma_mm2s_sg(void);
void _xdma_s2mm_sg(void);

volatile void const* _xdma_s2mm_simple(const size_t len);
volatile void const* _xdma_mm2s_simple(void const* mem, const size_t len);