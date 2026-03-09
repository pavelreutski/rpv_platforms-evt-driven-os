#pragma once

#include <stddef.h>

void _xdma_start(void);

/******************* sdcard DMA ***********************************/

volatile void const* _sdcdma_rdt(const size_t len);
volatile void const* _sdcdma_wrt(void const* mem, const size_t len);

/***************** sound blaster DMA ******************************/

void _sbdma_sgstop(void);

bool _sbdma_sgcmpltSignal(void);
bool _sbdma_sgbuserrSignal(void);

void *_sbdma_sgcmplt(const size_t len);

volatile void const* _sbdma_sgcyclic(const int seed, const size_t alloc_len, const size_t io_buff);

/************************ ethernet DMA ****************************/

void _ethdma_sgstop(void);

bool _ethdma_rxbuserrSignal(void);

void *_ethdma_rxsgcmplt(void *mem, const size_t mem_len, size_t *const trans_len);
volatile void const* _ethdma_rxsgcyclic(const size_t alloc_len, const size_t io_buff);