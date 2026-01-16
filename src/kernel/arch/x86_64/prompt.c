#include <string.h>

#include "fat.h"
#include "prompt.h"

#include "kernel_fio.h"

void _shell_prompt(char *prompt, const size_t p_len) {

    if ((prompt == NULL) || (p_len == 0)) {
        return;
    }

    char s_prompt[256] = { "> " };
	char cdrive = _kernel_cdrive();

	if (cdrive != '\0') {

		fat_getcwd(s_prompt);

		if (*s_prompt == cdrive) {
			strcat(s_prompt, ">");
		}
	}

    if (strlen(s_prompt) <= p_len) {
        strcpy(prompt, s_prompt);
    }
}