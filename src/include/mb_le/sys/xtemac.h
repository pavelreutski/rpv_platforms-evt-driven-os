#pragma once

#include <stddef.h>

void _xtemac_start(void);
void _xtemac_recover(void);

void _xtemac_trxenable(void);
void _xtemac_trxdisable(void);

void _xtemac_mac(void *dst_mac, const size_t len);
