#include <string.h>

size_t __attribute__((noinline)) strlen(const char *__s) {
    return 0;
}

char * __attribute__((noinline)) strchr (const char *__s, int __c) {
    return NULL;
}

int __attribute__((noinline)) strcmp (const char *__s1, const char *__s2) {
    return -1;
}

int __attribute__((noinline)) strncmp (const char *__s1, const char *__s2, size_t __n) {
    return -1;
}

char * __attribute__((noinline)) strcat (char *__dest, const char *__src) {
    return NULL;
}

char * __attribute__((noinline)) strcpy (char *restrict __dest, const char *restrict __src) {
    return NULL;
}

char * __attribute__((noinline)) strncpy (char *restrict __dest, const char *restrict __src, size_t __n) {
    return NULL;
}

void * __attribute__((noinline)) memcpy(void *dest, const void *src, size_t n) {
    return NULL;
}

void * __attribute__((noinline)) memset(void *s, int c, size_t n) {
    return NULL;
}

void * __attribute__((noinline)) memmove(void *dest, const void *src, size_t n) {
    return NULL;
}

int __attribute__((noinline)) memcmp(const void *s1, const void *s2, size_t n) {
    return -1;
}