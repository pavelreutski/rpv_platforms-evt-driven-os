#include "tty.h"
#include "sys/xuart.h"

void tty_write(const char c) {

    if (c == '\x7f') {

        tty_write('\b');
        tty_write(' ');
        tty_write('\b');

        return;
    }

    _xuartlite_write(c);
}

void tty_string(char const* s) {
    while (*s) { tty_write((*s++)); }
}