#pragma once

#include <stdint.h>

void fat_getcwd(char *cwd);
void fat_chdir(const char* cwd);

void fat_mount(char const* path);
void fat_unmount(char const* path);