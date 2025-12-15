#include <string.h>

#include "private/shell_cmd.h"

#define MAX_BUFFER_COMMANDS          (5)
#define MAX_COMMAND_LENGTH           (255)

#define COMMAND(cmd_ptr)             (cmd_ptr % MAX_BUFFER_COMMANDS)

static uint8_t CmdWndPtr         = 0;

static uint8_t WndCmds           = 0;
static uint8_t CmdWndBuffCounter = 0;

static char CmdWndBuffer[MAX_BUFFER_COMMANDS][MAX_COMMAND_LENGTH];

void push_newWndCmd(char *cmd) {

	if (!strlen(cmd)) return;

	char *buffer =
			CmdWndBuffer[COMMAND(CmdWndBuffCounter)];

	strcpy(buffer, cmd);

	CmdWndPtr = ++CmdWndBuffCounter;

	if (WndCmds < MAX_BUFFER_COMMANDS) WndCmds++;
}

bool get_wndCmd(char **cmd, cmd_stack_cursor_t cursor) {

	if (!(*cmd) || !WndCmds) return false;

	switch(cursor) {

		case CWND_NEXT_COMMAND: {

			if (!(CmdWndPtr ^ CmdWndBuffCounter) |
					!(CmdWndPtr ^ (CmdWndBuffCounter - 1)))
				return false;

			CmdWndPtr++;

		} break;

		case CWND_PREV_COMMAND: {

			if (!(CmdWndPtr ^ (CmdWndBuffCounter - WndCmds)))
				return false;

			CmdWndPtr--;

		} break;
		default: return false;
	}

	char *buffer =
			CmdWndBuffer[COMMAND(CmdWndPtr)];

	strcpy(*cmd, buffer);

	return true;
}