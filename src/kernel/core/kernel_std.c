#include <stddef.h>
#include <stdbool.h>

#include <stdlib.h>

long int atol (const char *__nptr) {

    const char *s = __nptr;

    if (s == NULL) {
        return 0;
    }

    while((*s == ' ') || 
            (*s == '\t') || 
                (*s == '\n')) { s++; }
    
    long int d = 0;
    bool is_neg = 0;

    if (*s == '+' || (*s == '-')) {

        is_neg = (*s == '-');
        s++;
    }

    while(*s) {

        const char c = *s;

        if ((c < '0') || (c > '9')) {
            break;
        }

        d *= 10;
        d += (c - '0');

        s++;
    }

    return is_neg ? (-d) : d;
}