#pragma once

#include <stdint.h>

enum {
    
	NO_EXEC,
	EXEC_EXTERNAL,
	EXEC_BUILT_IN
};

struct command_s {

    const char* text;
    uint8_t (*const handler)(char const* data, const int argc, const char **argv);
};

typedef struct command_s command_t;

#define _SHELL_COMMAND(name, callback) \
    __attribute__((used, section(".cmd_table"))) \
    static const command_t name = { #name, callback }
