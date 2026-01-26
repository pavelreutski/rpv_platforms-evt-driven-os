#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

void _xdma_start(void);

void _xdma_mm2s_sg(void);
void _xdma_s2mm_sg(void);

bool _xdma_mm2s_simple(void const* addr, size_t len);
bool _xdma_s2mm_simple(void const* addr, size_t len);