#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum {

	CWND_NEXT_COMMAND,
	CWND_PREV_COMMAND

} cmd_stack_cursor_t;

void push_newWndCmd(char *cmd);
bool get_wndCmd(char **cmd, cmd_stack_cursor_t cursor);
