#include <stddef.h>

#include "fat.h"
#include "kernel_stdio.h"

#include "utils/copy.h"

int cp_m(const int argc, const char **argv) {

    if (argc == 0) {
        _kernel_outString("no paramters given\n");
    } else {
        _kernel_outStringFormat("paramters given: %d\n", argc);
        for (int i = 0; i < argc; i++) {
            _kernel_outStringFormat("parameter#%d: %s\n", i, argv[i]);
        }
    }

    return 0;
}