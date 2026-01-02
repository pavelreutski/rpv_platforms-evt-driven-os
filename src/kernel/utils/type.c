#include "fat.h"
#include "kernel_stdio.h"

#include "utils/type.h"

#define MAX_FILE_VIEW_BUFFER                           1024

#define NO_SUCH_FILE_MSG                               ("No such file\n")
#define TOO_FEW_PARAMS_MSG                             ("File to open is not specified\n")

int type_main(const int argc, const char **argv) {

    if (argc == 0) {

        _kernel_outString(TOO_FEW_PARAMS_MSG);
        return -1;
    }

    char *path = (char *) argv[0];

    if (!fat_exists(path)) {

        _kernel_outString(NO_SUCH_FILE_MSG);
        return -1;
    }
    
    int fd = fat_fopen(path, FILE_READ);

    size_t b_read;
    char buffer[MAX_FILE_VIEW_BUFFER];

    while ((b_read = fat_fread(fd, buffer, sizeof(buffer) - 1)) > 0) {

        buffer[b_read] = '\0';
        _kernel_outString(buffer);
    }
        
    fat_fclose(fd);
    _kernel_outChar('\n');

    return 0;
}