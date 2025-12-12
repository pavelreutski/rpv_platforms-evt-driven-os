global _arch_memcpy, _arch_memset, _arch_memmove, _arch_memcmp

[section .text]

[bits 64]

; rdi <- dest
; rsi <- src
; rdx <- n

; rax -> dest

_arch_memcpy:
        mov     rax, rdi
        ret

; rdi <- s
; rsi <- c
; rdx <- n

; rax -> s

_arch_memset:
        mov     rax, rdi
        ret
        
; rdi <- dest
; rsi <- src
; rdx <- n

; rax -> dest 

_arch_memmove:
        mov     rax, rdi
        ret

; rdi <- s1
; rsi <- s2
; rdx <- n

; rax -> 0 when s1 = s2, 1 when s1 > s2, -1 when s1 < s2

_arch_memcmp:
        xor     rax, rax
        ret