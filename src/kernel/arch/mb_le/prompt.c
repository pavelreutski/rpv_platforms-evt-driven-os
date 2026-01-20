#include "prompt.h"

#define MAX_PROMPT_SIZE             (3)

void _shell_prompt(char *prompt, const size_t p_len) {

    if ((prompt == NULL) || (p_len < MAX_PROMPT_SIZE)) {
        return;
    }

    (*prompt++) = '#';
    (*prompt++) = ' ';

    *prompt = '\0';
}