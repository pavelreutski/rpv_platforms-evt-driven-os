#include <stddef.h>

#include "ff.h"

#include "fat.h"
#include "disk.h"

static volatile FATFS disk_fat;

void fat_mount(char const* path) {
    f_mount((FATFS *) &disk_fat, path, 1);
}

void fat_unmount(char const* path) {
    f_unmount(path);  
}

void fat_getcwd(char *cwd) {
    f_getcwd(cwd, 255);
}

void fat_chdir(char const* cwd) {
    f_chdir(cwd);
}