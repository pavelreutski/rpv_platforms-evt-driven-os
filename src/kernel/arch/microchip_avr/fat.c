#include "fat.h"

void fat_getcwd(char *cwd) { }

bool fat_exists(char const* path) { return false; }

bool fat_chdir(char const* cwd) { return false; }
bool fat_mkdir(char const* path) { return false; }
bool fat_unlink(char const* path) { return false; }

bool fat_mount(char const* path) { return false; }
bool fat_unmount(char const* path) { return false; }

bool fat_stat(char const* path, filinfo_t *fno) { return false; }

fat_error_code_t fat_getcode(void) { return FAT_IO_ERROR; }

void fat_fclose(int fd) { }
int fat_fopen(char const* path, file_access_t mode) { return -1; }

size_t fat_fread(int fd, void *buffer, size_t nread) { return 0; }
size_t fat_fwrite(int fd, void const* buffer, size_t nwrite) { return 0; }

void fat_ls(char const* path, void const* ls_ctx, 
    void (*nxt_file)(void const* ls_ctx, filinfo_t const* fi)) { }