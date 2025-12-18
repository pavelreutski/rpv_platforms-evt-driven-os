#include <stddef.h>

size_t __attribute__((noinline)) strlen(const char *__s) {

    extern size_t __attribute__((sysv_abi)) _arch_strlen(char const*);
    return _arch_strlen(__s);
}

char * __attribute__((noinline)) strchr (const char *__s, int __c) {

    extern char* __attribute__((sysv_abi)) _arch_strchr(char const*, int);
    return _arch_strchr(__s, __c);
}

int __attribute__((noinline)) strcmp (const char *__s1, const char *__s2) {

    extern int __attribute__((sysv_abi)) _arch_strcmp(char const*,  char const*);
    return _arch_strcmp(__s1, __s2);
}

int __attribute__((noinline)) strncmp (const char *__s1, const char *__s2, size_t __n) {

    extern int __attribute__((sysv_abi)) _arch_strncmp(char const*, char const*, size_t);
    return _arch_strncmp(__s1, __s2, __n);
}

char * __attribute__((noinline)) strcat (char *__dest, const char *__src) {

    extern char * __attribute__((sysv_abi)) _arch_strcat(char *__dest, const char *__src);
    return _arch_strcat(__dest, __src);
}

char * __attribute__((noinline)) strcpy (char *restrict __dest, const char *restrict __src) {

    extern char* __attribute__((sysv_abi)) _arch_strcpy(char *, char const*);
    return _arch_strcpy(__dest, __src);
}

char * __attribute__((noinline)) strncpy (char *restrict __dest, const char *restrict __src, size_t __n) {

    extern char* __attribute__((sysv_abi)) _arch_strncpy(char *, char const*, size_t __n);
    return _arch_strncpy(__dest, __src, __n);
}

void * __attribute__((noinline)) memcpy(void *dest, const void *src, size_t n) {

    extern void* __attribute__((sysv_abi)) _arch_memcpy(void *, void const*, size_t);
    return _arch_memcpy(dest, src, n);
}

void * __attribute__((noinline)) memset(void *s, int c, size_t n) {

    extern void* __attribute__((sysv_abi)) _arch_memset(void *, int, size_t);
    return _arch_memset(s, c, n);
}

void * __attribute__((noinline)) memmove(void *dest, const void *src, size_t n) {

    extern void* __attribute__((sysv_abi)) _arch_memmove(void *, void const*, size_t);
    return _arch_memmove(dest, src, n);
}

int __attribute__((noinline)) memcmp(const void *s1, const void *s2, size_t n) {

    extern int __attribute__((sysv_abi)) _arch_memcmp(void const*, void const*, size_t);
    return _arch_memcmp(s1, s2, n);
}

