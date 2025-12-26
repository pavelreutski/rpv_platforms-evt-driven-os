#pragma once

#include <stddef.h>
#include <stdint.h>

void _kernel_fio(void);

char _kernel_cdrive(void);
size_t _kernel_drives(char *drive_lt, size_t nlt);