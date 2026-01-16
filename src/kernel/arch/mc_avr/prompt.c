#include "prompt.h"

#define PROMPT_SIZE     (3)

void _shell_prompt(char *prompt, const size_t p_len) {

    if ((prompt == NULL) || (p_len < PROMPT_SIZE)) {
        return;
    }

    char *s = prompt;

    *(s++) = '>';
    *(s++) = ' ';

    *s = 0;
}