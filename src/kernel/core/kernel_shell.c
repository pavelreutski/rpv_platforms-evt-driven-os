#include <string.h>

#include <stdint.h>
#include <stdbool.h>

#include "shell.h"
#include "diskio.h"
#include "events.h"

#include "kernel.h"
#include "kernel_fio.h"
#include "kernel_stdio.h"
#include "kernel_exec.h"

#include "private/shell_cmd.h"

#define VER_MAJOR                                (3)
#define VER_MINOR                                (3)
#define VER_PATCH                                (0)

#define VER_REVISION							 ("alpha")

#define MAX_COMMANDS							 (20)
#define MAX_COMMAND_ARGS						 (10)
#define MAX_COMMAND_BUFFER                       (255)
#define MAX_COMMAND_PRMPT_BUFFER                 (255)

//// ---- Messages

#define NRDY_COMMAND_MSG						 ("Command not ready\n")

#define BAD_COMMAND_MSG                          ("Bad command or file name\n")
#define BAD_EXEC_FORMAT_MSG                      ("Bad executable image format\n")

#define PROG_PROCESS_DONE_MSG                    ("[COMMAND.COM]: Program and its process are Done\n")
#define PROG_PROCESS_RUNNING_MSG                 ("[COMMAND.COM]: Program process running in a background\n")

#define COMMAND_COM_VER_MSG                      ("Event Driven OS [Version %d.%d.%d-%s]\n")

#define CMD_SHELL_COPYRIGHT_MSG                  ("(C)RPV Co 2004-2025.\n")
#define CMD_SHELL_WELCOME_MSG                    ("RPVCo(R) Command Shell Program\n")

typedef enum {

	NO_EXEC,
	EXEC_EXTERNAL,
	EXEC_BUILT_IN

} exec_codes_t;

typedef struct {

	const char* cmd;
	uint8_t (*handler)(char* input, uint8_t nLength);

} command_t;

static uint8_t CmdBufferLength                          = 0;

// static char *CmdArgs[MAX_COMMAND_ARGS];

static char CmdBuffer[MAX_COMMAND_BUFFER];
static char CmdPrmptBuffer[MAX_COMMAND_PRMPT_BUFFER];

/*static const char *ExecLookupTbl[]                    = { ".\\",
														  "C:\\",
		                                                  "C:\\toolset\\",
														  "C:\\programs\\", (const char *) NULL };*/

//// Private internal calls

static void clear_cmdDisplayBuffer(void);

static void display_badCmdMsg(void);
static void display_cmdPromptLn(void);

static void fetch_wndCmd(uint8_t direction);

static void translate_userInput(void);
static uint8_t try_executeCommand(void);

static void onProg_processFinished(evt_data_t* evtData);

/* static void exec_ja(char* input, uint8_t nArgsLen,
		int (*executable)(int argc, const char **argv));
static void exec_f(char* input, uint8_t nArgsLen, const char *file); */

// static const char** parse_cmdArgs(char* input, int *argc, uint8_t nArgsLen);

// Command handlers

static uint8_t onVer_display(char* input, uint8_t nArgsLen);
static uint8_t onDisplay_cfg(char* input, uint8_t nArgsLen);
static uint8_t onVideoDevice_statQuery(char* input, uint8_t nArgsLen);

//// External Embedded Programs

static uint8_t onCopy_exec(char* input, uint8_t nArgsLen);
static uint8_t onType_exec(char* input, uint8_t nArgsLen);

//// Disk Operations

static uint8_t onDisk_info(char* input, uint8_t nArgsLen);
static uint8_t onDisk_mkDir(char* input, uint8_t nArgsLen);
static uint8_t onDiskFile_rm(char* input, uint8_t nArgsLen);
static uint8_t onDisk_chDir(char* input, uint8_t nArgsLen);
static uint8_t onSysDisk_eject(char* input, uint8_t nArgsLen);
static uint8_t onSysDisk_mount(char* input, uint8_t nArgsLen);
static uint8_t onDiskDir_display(char* input, uint8_t nArgsLen);

static command_t Commands[MAX_COMMANDS] =
		{
				{ "ver", onVer_display },
				{ "type", onType_exec },
				{ "copy", onCopy_exec },
				{ "display", onDisplay_cfg },
				{ "video", onVideoDevice_statQuery },
				{ "mount", onSysDisk_mount },
				{ "eject", onSysDisk_eject },
				{ "dir", onDiskDir_display },
				{ "cd", onDisk_chDir },
				{ "rm", onDiskFile_rm },
				{ "rmdir", onDiskFile_rm },
				{ "mkdir", onDisk_mkDir },
				{ "disks", onDisk_info }
		};

//// Never returns

void _shell_start() {

	memset(CmdBuffer, 0, sizeof(CmdBuffer));
	strncpy(CmdPrmptBuffer, "> ", MAX_COMMAND_PRMPT_BUFFER);

	/// Self event subscriptions

	_kernel_subscribe_kernel_evt(
			0xff, onProg_processFinished);

	_kernel_fio();
	
	_kernel_outLn();
	_kernel_outTab();

	_kernel_outString(CMD_SHELL_WELCOME_MSG);

	_kernel_outLn();
	_kernel_outTab();

	_kernel_outString(CMD_SHELL_COPYRIGHT_MSG);

	_kernel_outLn();
	_kernel_outLn();

	display_cmdPromptLn();

	while(true)
		translate_userInput();
}

static void onProg_processFinished(evt_data_t* evtData) {

	(void) evtData;

	_kernel_outString(PROG_PROCESS_DONE_MSG);
	display_cmdPromptLn();
}

static void translate_userInput() {

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

					if (!CmdBufferLength) break;

					CmdBufferLength--;
					_kernel_outChar(0x7f); // ASCII DEL

				} break;
				case '\n': {

					// Make command being in ASCIIZ format (NULL terminated)
					CmdBuffer[CmdBufferLength] = 0;

					push_newWndCmd(CmdBuffer);

					_kernel_outLn();

					uint8_t execType =
							try_executeCommand(); //// Try execute currently typed command

					CmdBufferLength ^=
							CmdBufferLength;

					if (execType ^ EXEC_EXTERNAL) // if anything apart from the external command execution -> display the command line prompt
						display_cmdPromptLn();

				} break;
				default: {

					if (!(key.code)) break;
					if (!(CmdBufferLength ^ MAX_COMMAND_BUFFER)) break;

					_kernel_outChar(key.code);
					CmdBuffer[CmdBufferLength++] = key.code;

				} break;
			}

		} break;
	}
}

/* static const char** parse_cmdArgs(char* input, int *argc, uint8_t nArgsLen) {

	uint8_t ipos = 0;

	*argc ^= *argc;
	*CmdArgs = input;

	if (!nArgsLen)
		return (const char**) CmdArgs;

	(*argc)++;

	do {

		if (input[ipos] ^ 0x20) continue;

		input[ipos] = 0;

		CmdArgs[*argc] =
				input + ipos + 1; //// Argument starts at next character position

		(*argc)++;

	} while(++ipos < nArgsLen);

	return (const char**) CmdArgs;
}*/

static uint8_t try_executeCommand(void) {

	if (!CmdBufferLength) return NO_EXEC;

	char *argsInput = strchr(CmdBuffer, ' ');

	argsInput = !argsInput ?
			CmdBuffer + CmdBufferLength : argsInput + 1;

	uint8_t nArgsLen = strlen(argsInput);

	if (nArgsLen)
		*(argsInput - 1) = 0;

	// if (strstr(CmdBuffer, ".com") ||
	// 		strstr(CmdBuffer, ".COM")) {

	// 	// External executable

	// 	char *execBuffer = argsInput + nArgsLen + 1; // Ptr to next byte right after the args input + NULL termination byte
	// 	const char **execLookupTbl = ExecLookupTbl;

	// 	do {

	// 		strcpy(execBuffer, *execLookupTbl);
	// 		strcat(execBuffer, CmdBuffer);

	// 		if (!_kernel_f_is_exist(execBuffer)) continue;

	// 		exec_f(argsInput, nArgsLen, execBuffer);
	// 		return EXEC_EXTERNAL;

	// 	} while (*(++execLookupTbl));

	// 	displayBadCmdMsg();
	// 	return NO_EXEC;
	// }

	// Seek for internal command input

	for (uint8_t cmdId = 0; cmdId < MAX_COMMANDS; cmdId++) {

		command_t *command = &Commands[cmdId];

		if (command -> cmd == NULL) continue;

		if (!strcmp(CmdBuffer, command -> cmd))
			return (command -> handler)(argsInput, nArgsLen);
	}

	display_badCmdMsg();

	return NO_EXEC;
}

static void clear_cmdDisplayBuffer(void) {
	for (uint8_t cmdCh = 0; cmdCh < CmdBufferLength; cmdCh++)
		_kernel_outChar(0x7f); // ASCII DEL
}

static void display_cmdPromptLn(void) {
	_kernel_outString(CmdPrmptBuffer);
}

static void display_badCmdMsg(void) {
	_kernel_outString(BAD_COMMAND_MSG);
}

static void fetch_wndCmd(uint8_t direction) {

	char *cmd = CmdBuffer;
	if (!get_wndCmd(&cmd, direction)) return;

	clear_cmdDisplayBuffer();

    CmdBufferLength = strlen(cmd);
    _kernel_outString(cmd);
}

//// Command handlers

static uint8_t onVer_display(char* input, uint8_t nArgsLen) {

	(void) input;
	(void) nArgsLen;

	_kernel_outStringFormat(
		COMMAND_COM_VER_MSG, 
			VER_MAJOR, VER_MINOR, VER_PATCH, VER_REVISION);

	return EXEC_BUILT_IN;
}

static uint8_t onDisplay_cfg(char* input, uint8_t nArgsLen) {

	(void) input;
	(void) nArgsLen;

	/*int argc;
	const char **argv =
			parseCmdArgs(input, &argc, nArgsLen);

	DisplayOpSetMode(argc, argv);*/

	_kernel_outString(NRDY_COMMAND_MSG);
	return EXEC_BUILT_IN;
}

static uint8_t onVideoDevice_statQuery(char* input, uint8_t nArgsLen) {

	(void) input;
	(void) nArgsLen;

	// VideoOpDeviceStateDisplay();
	_kernel_outString(NRDY_COMMAND_MSG);
	return EXEC_BUILT_IN;
}

//// Disk Operations

static uint8_t onDisk_info(char* input, uint8_t nArgsLen) {

	(void) input;
	(void) nArgsLen;

	size_t n_disks = get_disks();

	for (size_t i = 0; i < n_disks; i++) {

		char dsk_info[255];
		get_diskInfo(i, dsk_info, sizeof(dsk_info));

		_kernel_outStringFormat("%s\n", dsk_info);		
	}

	return EXEC_BUILT_IN;
}

static uint8_t onDiskFile_rm(char* input, uint8_t nArgsLen) {

	(void) input;
	(void) nArgsLen;

	// DiskOpRmFile(input);

	_kernel_outString(NRDY_COMMAND_MSG);
	return EXEC_BUILT_IN;
}

static uint8_t onSysDisk_eject(char* input, uint8_t nArgsLen) {

	(void) input;
	(void) nArgsLen;

	// DiskOpUnmountSysDisk();

	_kernel_outString(NRDY_COMMAND_MSG);
	return EXEC_BUILT_IN;
}

static uint8_t onSysDisk_mount(char* input, uint8_t nArgsLen) {

	(void) input;
	(void) nArgsLen;

	// DiskOpMountSysDisk();

	_kernel_outString(NRDY_COMMAND_MSG);
	return EXEC_BUILT_IN;
}

static uint8_t onDisk_mkDir(char* input, uint8_t nArgsLen) {

	(void) input;
	(void) nArgsLen;

	// DiskOpMkDir(input);

	_kernel_outString(NRDY_COMMAND_MSG);
	return EXEC_BUILT_IN;
}

static uint8_t onDisk_chDir(char* input, uint8_t nArgsLen) {

	(void) input;
	(void) nArgsLen;

	// DiskOpChDir(input);

	_kernel_outString(NRDY_COMMAND_MSG);
	return EXEC_BUILT_IN;
}

static uint8_t onDiskDir_display(char* input, uint8_t nArgsLen) {

	(void) input;
	(void) nArgsLen;

	/*int argc = 0;

	const char **argv =
				parseCmdArgs(input, &argc, nArgsLen);

	DiskOpDirDisplay(argc, argv);*/

	_kernel_outString(NRDY_COMMAND_MSG);
	return EXEC_BUILT_IN;
}

//// Embedded programs exec host

static uint8_t onCopy_exec(char* input, uint8_t nArgsLen) {

	(void) input;
	(void) nArgsLen;

	// exec_ja(input, nArgsLen, copy_main);

	_kernel_outString(NRDY_COMMAND_MSG);
	return EXEC_EXTERNAL;
}

static uint8_t onType_exec(char* input, uint8_t nArgsLen) {

	(void) input;
	(void) nArgsLen;

	// exec_ja(input, nArgsLen, type_main);

	_kernel_outString(NRDY_COMMAND_MSG);
	return EXEC_EXTERNAL;
}

//// External programs execution

/* static void exec_f(char* input, uint8_t nArgsLen, const char *file) {

	int (*laddr)(int, const char **) =
					(int (*)(int, const char **)) _kernel_load_exec(file);

	if (!laddr) {

		_kernel_out_str_ln(BAD_EXEC_FORMAT_MSG);
		return;
	}

	exec_ja(input, nArgsLen, laddr);
}

static void exec_ja(char* input, uint8_t nArgsLen,
		int (*executable)(int argc, const char **argv)) {

	int argc;
	const char** argv =
			parseCmdArgs(input, &argc, nArgsLen);

	_kernel_exec_f(executable, argc, argv);
}*/ 
