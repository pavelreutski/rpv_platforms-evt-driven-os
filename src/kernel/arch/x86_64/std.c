#include <stddef.h>
#include <stdbool.h>

#include <stdlib.h>
#include <string.h>

unsigned long strtoul(const char *__nptr, char **__endptr, int __base) {

    unsigned long d = 0;
    
    if (__endptr != NULL) {
        *__endptr = (char *) __nptr;
    }

    if ((__nptr == NULL) || (__base != 16)) {
        return d;
    }
    
    char *s = (char *) __nptr;

    if ((*s) == '0') {

        s++;
        if ((*s == 'x') || (*s == 'X')) {
            s++;
        }
    }

    char c;
    while ((c = (*s))) {

        int digit;

        if ((c >= '0') && (c <= '9')) {          
            digit = (c - '0');
        } else if (((c >= 'a') && (c <= 'f'))) {
            digit = (c - 'a' + 10);
        } else if (((c >= 'A') && (c <= 'F'))) {
            digit = (c - 'A' + 10);
        } else {
            break;
        }
        
        s++;

        d <<= 4;
        d += digit;
    }
    
    if (__endptr != NULL) {
        *__endptr = s;
    }

    return d;
}

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