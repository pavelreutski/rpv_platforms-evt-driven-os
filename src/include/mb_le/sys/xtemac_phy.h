#pragma once

#include <stdint.h>
#include <stdbool.h>

void xtemac_phy(void);

bool xtemac_phylink(void);
void xtemac_phyid(char *s, const uint8_t len);