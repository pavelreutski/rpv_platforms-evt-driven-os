#pragma once

#include <stdint.h>
#include <stdbool.h>

#define MAX_FILE_NAME           (256)

typedef struct {

    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    uint8_t day;
    uint8_t month;
    uint16_t year;

} fildatetime_t;

typedef struct {

    fildatetime_t create_time;
    fildatetime_t modify_time;

    union {
        uint8_t a8;
        struct {
            bool r_only    : 1;
            bool archive   : 1;
            bool system    : 1;
            bool hidden    : 1;
            bool directory : 1;
        } flags;

    } attrib;    

    size_t filesize;

    char full_name[MAX_FILE_NAME];

} filinfo_t;

void fat_getcwd(char *cwd);

bool fat_exists(char const* path);

bool fat_chdir(char const* cwd);
bool fat_mkdir(char const* path);

bool fat_mount(char const* path);
bool fat_unmount(char const* path);

void fat_ls(char const* path, void const* ls_ctx, void (*nxt_file)(void const* ls_ctx, filinfo_t const* fi));