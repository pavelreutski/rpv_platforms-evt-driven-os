#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum : uint8_t {

    SPEED_10MBPS,
    SPEED_100MBPS

} phy_speed_t;

typedef enum : uint8_t {

    LINK_HALF_DUPLEX,
    LINK_FULL_DUPLEX

} phy_duplex_t;

typedef struct {

    phy_speed_t speed;
    phy_duplex_t link;

} phylink_t;

void _xtemac_phy(void);
void _xtemac_phyReset(void);

bool _xtemac_phylinkSgl(void);
bool _xtemac_phylink(phylink_t *lk);

uint32_t _xtemac_phyid(void);
uint16_t _xtemac_phyreg(uint8_t addr);