#pragma once

#include <stddef.h>

#define XTEMAC_MTU                          (1536)
#define XTEMAC_MAC_ADDRLEN                  (6)

void _xtemac_start(void);
void _xtemac_recover(void);

void _xtemac_trxenable(void);
void _xtemac_trxdisable(void);

void _xtemac_mac(void *dst_mac, const size_t len);
