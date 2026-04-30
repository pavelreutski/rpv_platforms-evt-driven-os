#pragma once

#include <stddef.h>

bool _sec_storeread(void *blck, const size_t blck_size);
bool _sec_storewrite(void const* blck, const size_t blck_size);