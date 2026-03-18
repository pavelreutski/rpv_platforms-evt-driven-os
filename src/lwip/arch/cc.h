#pragma once

#include <stdint.h>
#include <stddef.h>

/* Standard types for lwIP */
typedef uint8_t  u8_t;
typedef int8_t   s8_t;
typedef uint16_t u16_t;
typedef int16_t  s16_t;
typedef uint32_t u32_t;
typedef int32_t  s32_t;

/* Byte order macros */
#define LWIP_PLATFORM_BYTESWAP 1
#define LWIP_PLATFORM_HTONS(x) (((u16_t)(x) << 8) | ((u16_t)(x) >> 8))
#define LWIP_PLATFORM_HTONL(x) \
    (((u32_t)(x) << 24) | (((u32_t)(x) << 8) & 0x00FF0000) | \
     (((u32_t)(x) >> 8) & 0x0000FF00) | ((u32_t)(x) >> 24))

/* Diagnostic macros */
#define LWIP_PLATFORM_DIAG(x)  do { } while(0)
#define LWIP_PLATFORM_ASSERT(x) do { while(1); } while(0)

/* Critical section macros for NO_SYS (do nothing) */
#define SYS_ARCH_DECL_PROTECT(x) u32_t x
#define SYS_ARCH_PROTECT(x)   (void)x
#define SYS_ARCH_UNPROTECT(x)  (void)x