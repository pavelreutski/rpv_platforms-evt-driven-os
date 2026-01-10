#pragma once

#include <stdint.h>
#include <stdbool.h>

enum cmd_cursor_e {
	
	CWND_NEXT_COMMAND,
	CWND_PREV_COMMAND
};

typedef enum cmd_cursor_e cmd_stack_cursor_t;

void push_newWndCmd(char *cmd);
bool get_wndCmd(char **cmd, cmd_stack_cursor_t cursor);
