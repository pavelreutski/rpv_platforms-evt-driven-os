#include <stddef.h>

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