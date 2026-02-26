#pragma once

#include <stdint.h>
#include <stdbool.h>

void _xtemac_phy(void);

bool _xtemac_phylink(void);
uint32_t _xtemac_phyid(void);