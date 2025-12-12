#pragma once

#include <stdint.h>

typedef enum {

	KEYB_INPUT_PENDING                 = 0x01,
	FILE_IO_PENDING                    = 0x02

} kernel_context_flags_t;

void _kernel_clear_context_flags(uint16_t clrFlags);
void _kernel_context_flags(uint16_t setFlags, uint16_t **ctxFlags);