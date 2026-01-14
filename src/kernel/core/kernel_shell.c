#include <string.h>
#include <stdlib.h>

#include <stdint.h>
#include <stdbool.h>

#include "fat.h"
#include "shell.h"
#include "events.h"

#include "kernel.h"
#include "kernel_fio.h"
#include "kernel_stdio.h"

#include "private/command.h"
#include "private/shell_cmd.h"

#define MAX_COMMAND_ARGS						 (10)
#define MAX_COMMAND_BUFFER                       (255)

// ---- Messages

#define BAD_COMMAND_MSG                          ("Bad command or file name\n")
#define BAD_EXEC_FORMAT_MSG                      ("Bad executable image format\n")

#define PROG_PROCESS_DONE_MSG                    ("[COMMAND.COM]: Program and its process are Done\n")
#define PROG_PROCESS_RUNNING_MSG                 ("[COMMAND.COM]: Program process running in a background\n")

#define CMD_SHELL_COPYRIGHT_MSG                  ("(C)RPV Co 2004-2025.\n")
#define CMD_SHELL_WELCOME_MSG                    ("RPVCo(R) Command Shell Program\n")

static char *cmd_args[MAX_COMMAND_ARGS];
static char cmd_buffer[MAX_COMMAND_BUFFER];

static uint8_t cmd_length                         = 0;

// private internal calls

static void display_cmdPrompt(void);
static void fetch_wndCmd(uint8_t direction);

static uint8_t exec_command(void);

static void onProc_exit(evt_data_t* evtData);
static void parse_cmdArgs(char* input, int length, char **argv, int *argc);

extern command_t __cmd_table[];
extern command_t __end_cmd_table[];

// never returns

void _shell_start() {

	memset(cmd_buffer, 0, sizeof(cmd_buffer));

	/// Self event subscriptions

	_kernel_subEvt(0xff, onProc_exit);
	
	_kernel_outLn();
	_kernel_outTab();

	_kernel_outString(CMD_SHELL_WELCOME_MSG);

	_kernel_outLn();
	_kernel_outTab();

	_kernel_outString(CMD_SHELL_COPYRIGHT_MSG);

	_kernel_outLn();
	_kernel_outLn();

	_kernel_fio();
	display_cmdPrompt();

	while(true) {

		console_key_t key;
		_kernel_getKey(&key);

		switch(key.scan_code) {

			// Handle scan codes for the control keys e.g. arrow keys

			case 0x75: // Arrow UP
				fetch_wndCmd(CWND_PREV_COMMAND); break;
			case 0x72:// Arrow DOWN
				fetch_wndCmd(CWND_NEXT_COMMAND); break;
			default: {

				// Handle ASCII code

				switch(key.code) {

					case '\b': {

						if (!cmd_length) break;

						cmd_length--;
						_kernel_outChar(0x7f); // ASCII DEL

					} break;

					case '\n': {

						// make command being in ASCIIZ format (NULL terminated)
						cmd_buffer[cmd_length] = 0;

						push_newWndCmd(cmd_buffer);

						_kernel_outLn();
						uint8_t r_code = exec_command(); // try execute currently typed command

						cmd_length ^= cmd_length;

						if (r_code ^ EXEC_EXTERNAL) // if anything apart from the external command execution -> display the command line prompt
							display_cmdPrompt();

					} break;

					default: {

						if (!(key.code)) break;
						if (!(cmd_length ^ MAX_COMMAND_BUFFER)) break;

						_kernel_outChar(key.code);
						cmd_buffer[cmd_length++] = key.code;

					} break;
				}

			} break;
		}
	}
}

static void onProc_exit(evt_data_t* evtData) {

	(void) evtData;

	_kernel_outString(PROG_PROCESS_DONE_MSG);
	display_cmdPrompt();
}

static void parse_cmdArgs(char* input, int length, char **argv, int *argc) {		

	if (length == 0) {

		*argc = 0;
		return;
	}

	int i = 0;
	int arg_c = 1;

	char *arg_s = input;
	char **args_v = argv;

	*args_v = arg_s;
	args_v++;

	do {

		if (*arg_s == ' ') {

			*arg_s = '\0';
			*args_v = (arg_s + 1); // argument starts at next character position

			arg_c++;
			args_v++;
		}

		arg_s++;

	} while((++i) < length);

	*argc = arg_c;
}

static uint8_t exec_command(void) {

	if (!cmd_length) return NO_EXEC;

	char *args_input = strchr(cmd_buffer, ' ');

	args_input = (args_input == NULL) ?
			(cmd_buffer + cmd_length) : (args_input + 1);

	uint8_t args_len = strlen(args_input);

	if (args_len > 0) {
		*(args_input - 1) = '\0';
	}

	// seek for internal command input

	for (command_t *cmd = (command_t *) &__cmd_table; 
			cmd != (command_t *) &__end_cmd_table; cmd++) {

		if ((cmd -> text != NULL) &&
				strcmp(cmd_buffer, cmd -> text) == 0) {

			int argc;
			char **argv = cmd_args;

			parse_cmdArgs(args_input, args_len, argv, &argc);

			return (cmd -> handler)(args_input, argc, (const char **) argv);
		}
	}

	_kernel_outString(BAD_COMMAND_MSG);

	return NO_EXEC;
}

static void display_cmdPrompt(void) {

	char s_prompt[256] = { "> " };
	char cdrive = _kernel_cdrive();

	if (cdrive != '\0') {

		fat_getcwd(s_prompt);

		if (*s_prompt == cdrive) {
			strcat(s_prompt, ">");
		}
	}

	_kernel_outString(s_prompt);
}

static void fetch_wndCmd(uint8_t direction) {

	char *cmd = cmd_buffer;
	if (!get_wndCmd(&cmd, direction)) { 
		return;
	}

	// clear cmd prompt

	for (size_t i = 0; i < cmd_length; i++) {
		_kernel_outChar(0x7f); // ascii del
	}

    cmd_length = strlen(cmd);
    _kernel_outString(cmd);
}
