#pragma once

#include <stdbool.h>

void _kernel_jentry(char const* entry);
bool _kernel_jnxtentry(char *entry, const size_t max_len);
