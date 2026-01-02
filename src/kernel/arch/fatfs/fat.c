#include <string.h>
#include <stddef.h>

#include "ff.h"

#include "fat.h"
#include "disk.h"

#define MAX_OPEN_FILES          (10)
#define FILE_YEAR_BASE          (1980)

#define FILE_YEAR(year_mod)     (year_mod + FILE_YEAR_BASE)

static volatile uint8_t files   = 0;

static volatile FATFS disk_fat;
static volatile fat_error_code_t error_code;

static volatile FIL file_desc[MAX_OPEN_FILES];
static volatile FIL *open_files[MAX_OPEN_FILES]; 

static void set_fatErrCode(FRESULT fs_result);

static void to_filinfo(filinfo_t *fil, FILINFO const* fno);
static void to_filDateTime(fildatetime_t *fil_dt, const WORD fs_date, const WORD fs_time);

fat_error_code_t fat_getcode(void) {
    return error_code;
}

bool fat_mount(char const* path) {

    set_fatErrCode(
        f_mount((FATFS *) &disk_fat, path, 1));

    if (error_code == FAT_OK) {

        files = 0;
        memset((FIL *) open_files, 0, sizeof(FIL *) * MAX_OPEN_FILES);
    }

    return error_code == FAT_OK;
}

bool fat_unmount(char const* path) {
    set_fatErrCode(f_unmount(path));
    return error_code == FAT_OK;
}

bool fat_chdir(char const* cwd) {
    set_fatErrCode(f_chdir(cwd));
    return error_code == FAT_OK;
}

bool fat_mkdir(char const* path) {
    set_fatErrCode(f_mkdir(path));
    return error_code == FAT_OK;
}

bool fat_unlink(char const* path) {
    set_fatErrCode(f_unlink(path));
    return error_code == FAT_OK;
}

bool fat_exists(char const* path) {

    FILINFO fno;
    set_fatErrCode(f_stat(path, &fno));
    return error_code == FAT_OK;
}

void fat_getcwd(char *cwd) {
    
    set_fatErrCode(f_getcwd(cwd, sizeof(TCHAR) * 255));

    if (error_code != FAT_OK) {
        *cwd = '\0';
    }
}

bool fat_stat(char const* path, filinfo_t *fno) {

    FILINFO fs_fno;
    set_fatErrCode(f_stat(path, &fs_fno));

    if (error_code == FAT_OK) {
        to_filinfo(fno, &fs_fno);
    }

    return (error_code == FAT_OK);
}

void fat_ls(char const* path, void const* ls_ctx, 
                void (*nxt_file)(void const* ls_ctx, filinfo_t const* fi)) {

    DIR dp;
    FILINFO finfo;

    set_fatErrCode(f_findfirst(&dp, &finfo, path, "*")); // search all

    while ((error_code == FAT_OK) && 
                (*finfo.fname != '\0')) {

        filinfo_t fil;
        to_filinfo(&fil, &finfo);

        nxt_file(ls_ctx, &fil);

        set_fatErrCode(f_findnext(&dp, &finfo));
    }

    f_closedir(&dp);
}

void fat_fclose(int fd) {

    if (fd < 0 || fd >= MAX_OPEN_FILES) {
        return;
    }

    FIL *fp = (FIL *) open_files[fd];
    set_fatErrCode(f_close(fp));

    files--;
    open_files[fd] = NULL;
}

int fat_fopen(char const* path, file_access_t mode) {

    if (files == MAX_OPEN_FILES) {
        return -1;
    }

    size_t i = 0;
    FIL *fp = *((FIL **) open_files);

    for (i = 0; 
            (fp != NULL) && (i < MAX_OPEN_FILES); i++, fp = (FIL *) open_files[i]) { }

    fp = (FIL *) &file_desc[i];
    set_fatErrCode(f_open(fp, path, mode));

    if (error_code != FAT_OK) {
        return -1;
    }

    files++;
    open_files[i] = fp;
    
    return i;
}

size_t fat_fread(int fd, void *buffer, size_t nread) {

    if (fd < 0 || fd >= MAX_OPEN_FILES) {
        return 0;
    }

    FIL *fp = (FIL *) open_files[fd];

    UINT rd;
    set_fatErrCode(f_read(fp, buffer, nread, &rd));

    return rd;
}

size_t fat_fwrite(int fd, void const* buffer, size_t nwrite) {

    if (fd < 0 || fd >= MAX_OPEN_FILES) {
        return 0;
    }

    FIL *fp = (FIL *) open_files[fd];

    UINT wr;
    set_fatErrCode(f_write(fp, buffer, nwrite, &wr));

    return wr;
}

static void set_fatErrCode(FRESULT fs_result) {

    switch (fs_result) {

        case FR_OK: {
            error_code = FAT_OK; } break;
        case FR_DENIED: {
            error_code = FAT_ACCESS_DENIED; } break;

        case FR_DISK_ERR: {
            error_code = FAT_IO_ERROR; } break;

        case FR_NO_PATH:
        case FR_NO_FILE: {
            error_code = FAT_NOT_FOUND; } break;
    
        default: {
            error_code = FAT_ERR_NOTSET; } break;
    }
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

static void to_filinfo(filinfo_t *fil, FILINFO const* fno) {

    fil -> filesize = fno -> fsize;

    fil -> attrib.flags.system = ((fno -> fattrib & AM_SYS) != 0);
    fil -> attrib.flags.hidden = ((fno -> fattrib & AM_HID) != 0);
                
    fil -> attrib.flags.r_only = ((fno -> fattrib & AM_RDO) != 0);
    fil -> attrib.flags.archive = ((fno -> fattrib & AM_ARC) != 0);
    fil -> attrib.flags.directory = ((fno -> fattrib & AM_DIR) != 0);

    to_filDateTime(&fil -> modify_time, fno -> fdate, fno -> ftime);
    to_filDateTime(&fil -> create_time, fno -> crdate, fno -> crtime);

    strcpy(fil -> full_name, fno -> fname);
}