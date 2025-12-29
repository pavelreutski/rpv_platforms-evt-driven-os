#include <string.h>
#include <stddef.h>

#include "ff.h"

#include "fat.h"
#include "disk.h"

#define FILE_YEAR_BASE          (1980)

#define FILE_YEAR(year_mod)     (year_mod + FILE_YEAR_BASE)

static volatile FATFS disk_fat;

static void to_filDateTime(fildatetime_t *fil_dt, const WORD fs_date, const WORD fs_time);

bool fat_mount(char const* path) {
    return f_mount((FATFS *) &disk_fat, path, 1) == FR_OK;
}

bool fat_unmount(char const* path) {
    return f_unmount(path) == FR_OK;
}

bool fat_chdir(char const* cwd) {
    return f_chdir(cwd) == FR_OK;
}

bool fat_mkdir(char const* path) {
    return f_mkdir(path) == FR_OK;
}

bool fat_exists(char const* path) {

    FILINFO fno;
    return f_stat(path, &fno) == FR_OK;
}

void fat_getcwd(char *cwd) {

    if (f_getcwd(cwd, sizeof(TCHAR) * 255) != FR_OK) {
        *cwd = '\0';
    }
}

void fat_ls(char const* path, void const* ls_ctx, 
                void (*nxt_file)(void const* ls_ctx, filinfo_t const* fi)) {

    DIR dp;
    FILINFO finfo;

    FRESULT s_token = f_findfirst(&dp, &finfo, path, "*"); // search all

    while ((s_token == FR_OK) && 
                (*finfo.fname != '\0')) {

        filinfo_t fil;

        fil.filesize = finfo.fsize;

        fil.attrib.flags.system = ((finfo.fattrib & AM_SYS) != 0);
        fil.attrib.flags.hidden = ((finfo.fattrib & AM_HID) != 0);
                
        fil.attrib.flags.r_only = ((finfo.fattrib & AM_RDO) != 0);
        fil.attrib.flags.archive = ((finfo.fattrib & AM_ARC) != 0);
        fil.attrib.flags.directory = ((finfo.fattrib & AM_DIR) != 0);

        to_filDateTime(&fil.modify_time, finfo.fdate, finfo.ftime);
        to_filDateTime(&fil.create_time, finfo.crdate, finfo.crtime);
        
        strcpy(fil.full_name, finfo.fname);

        nxt_file(ls_ctx, &fil);

        s_token = f_findnext(&dp, &finfo);
    }

    f_closedir(&dp);
}

static void to_filDateTime(fildatetime_t *fil_dt, const WORD fs_date, const WORD fs_time) {    

    uint16_t year_mod = ((fs_date >> 9) & 0x7F); // year since 1980 i.e. 1980 + year_mod

    fil_dt -> year = FILE_YEAR(year_mod);

    fil_dt -> day = (fs_date & 0x1F);
    fil_dt -> month = ((fs_date >> 5) & 0x0F);    

    fil_dt -> hour = ((fs_time >> 11) & 0x1F);
    fil_dt -> minute = ((fs_time >> 5) & 0x3F);
    fil_dt -> second = ((fs_time & 0x3F) << 1); // sec * 2
}