#include <stddef.h>
#include <string.h>

#include "fat.h"
#include "kernel_stdio.h"

#include "utils/cp.h"

#define FILE_BUFFER                                 (2048)

#define FILE_WRITE_FAILED_MSG                       ("write failed")

#define FILE_OPEN_READ_FAILED_MSG                   ("unable open file to read")
#define FILE_OPEN_WRITE_FAILED_MSG                  ("unable open file for write")

#define NO_SUCH_FILE_MSG                            ("No such file")
#define TOO_FEW_PARAMS_MSG                          ("Too few parameters specified\n")

static void close_fdesc(int sfd, int dfd);

int cp_m(const int argc, const char **argv) {

    if (argc < 2) {

        _kernel_outString(TOO_FEW_PARAMS_MSG);
        return -1;
    }

    char *src_p = (char *) argv[0];
    char *dst_p = (char *) argv[1];

    if (!fat_exists(src_p)) {

        _kernel_outStringFormat("source: %s\n", NO_SUCH_FILE_MSG);
        return -1;
    }

    int f_cp = 0;
    int sfd = -1, dfd = -1;
    if ((sfd = fat_fopen(src_p, FILE_READ)) < 0) {

        _kernel_outStringFormat("source: %s\n", FILE_OPEN_READ_FAILED_MSG);
        return -1;
    }

    if ((dfd = fat_fopen(dst_p, (FILE_CREATE | FILE_WRITE))) < 0) {

        close_fdesc(sfd, dfd);
        _kernel_outStringFormat("dest: %s\n", FILE_OPEN_WRITE_FAILED_MSG);

        return -1;
    }

    size_t b_read;
    uint8_t f_buffer[FILE_BUFFER];

    while((b_read = fat_fread(sfd, f_buffer, sizeof(f_buffer))) > 0) {

        if (fat_fwrite(dfd, f_buffer, b_read) != b_read) {

            close_fdesc(sfd, dfd);
            _kernel_outStringFormat("dest: %s\n", FILE_WRITE_FAILED_MSG);

            return -1;
        }
    }

    f_cp = 1;
    _kernel_outStringFormat("%d file(s) copied\n", f_cp);

    close_fdesc(sfd, dfd);
    return 0;
}

static void close_fdesc(int sfd, int dfd) {

    fat_fclose(sfd);
    fat_fclose(dfd);
}