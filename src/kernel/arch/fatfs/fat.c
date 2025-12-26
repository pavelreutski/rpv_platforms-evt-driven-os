#include <stddef.h>

#include "ff.h"
#include "fat.h"

void fat_mount(void) {
    f_mount(NULL, NULL, 0);
}

void fat_chdir(void) {
    f_chdir("");
}