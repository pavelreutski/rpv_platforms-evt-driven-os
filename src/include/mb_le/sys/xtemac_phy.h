#pragma once

#include <stdint.h>
#include <stdbool.h>

void _xtemac_phy(void);
void _xtemac_phyReset(void);

bool _xtemac_phylink(void);
bool _xtemac_phylinkSgl(void);

uint32_t _xtemac_phyid(void);

uint16_t _xtemac_phyreg(uint8_t addr);