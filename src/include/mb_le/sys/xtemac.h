#pragma once

#include <stdint.h>

void _xtemac_start(void);

void _xtemac_rxdisable(void);
void _xtemac_rxenable(void);

void _xtemac_id(char *s, const uint8_t len);