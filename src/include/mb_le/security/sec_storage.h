#pragma once

#include <stdint.h>
#include <stddef.h>

bool _sec_storeread(uint8_t blck_uid, void *blck, const size_t blck_size);
bool _sec_storewrite(uint8_t blck_uid, void const* blck, const size_t blck_size);