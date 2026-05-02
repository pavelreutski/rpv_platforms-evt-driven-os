#pragma once

#include <stddef.h>
#include <stdint.h>

void _sec_prngseed(uint32_t seed);
void _sec_genprngseq(void *seq, const size_t seq_size);