#include <string.h>

#include "fat.h"
#include "console.h"
#include "monitor.h"

#include "kernel_stdio.h"

#include "utils/type.h"

#define MAX_FILE_VIEW_BUFFER                           1024

#define NO_SUCH_FILE_MSG                               ("No such file\n")
#define TOO_FEW_PARAMS_MSG                             ("File to open is not specified\n")

enum {
    HEX_VIEW,
    TEXT_VIEW
};

int type_m(const int argc, const char **argv) {

    if (argc == 0) {

        _kernel_outString(TOO_FEW_PARAMS_MSG);
        return -1;
    }

    char *path = (char *)((argc > 1) ? argv[1] : argv[0]);

    uint8_t view_kind = 
        (argc > 1 && !strcmp(argv[0], "hex")) ? HEX_VIEW : TEXT_VIEW;

    if (!fat_exists(path)) {

        _kernel_outString(NO_SUCH_FILE_MSG);
        return -1;
    }
    
    int fd = fat_fopen(path, FILE_READ);

    size_t b_read;
    char buffer[MAX_FILE_VIEW_BUFFER];

    while ((b_read = fat_fread(fd, buffer, sizeof(buffer) - 1)) > 0) {

        switch(view_kind) {

            case HEX_VIEW: {
                
                con_clear();
                hex_monitor(buffer, b_read);
            } break;

            case TEXT_VIEW: {

                buffer[b_read] = '\0';
                text_monitor(buffer);
            }
        }
    }
        
    fat_fclose(fd);
    _kernel_outChar('\n');

    return 0;
}