global _arch_memcpy, _arch_memset, _arch_memmove, _arch_memcmp
global _arch_strcpy, _arch_strncpy, _arch_strcmp, _arch_strncmp, _arch_strchr

[section .text]

[bits 64]

_arch_strchr:
        xor     rax, rax
        ret

; rdi <- s1
; rsi <- s2

; rax -> 0 when s1 = s2, > 0 when s1 > s2, < 0 when s1 < s2

_arch_strcmp:
        xor     rax, rax
        ret

; rdi <- s1
; rsi <- s2
; rdx <- n

; rax -> 0 when s1 = s2, > 0 when s1 > s2, < 0 when s1 < s2

_arch_strncmp:
        xor     rax, rax
        ret

; rdi <- dest
; rsi <- src
; rdx <- n

; rax -> dest

_arch_strncpy:

        cld
        mov     rax, rdi
        mov     rcx, rdx

        or      rcx, rcx
        jnz     .strn_cpy_loop

        ret

.strn_cpy_loop:

        lodsb
        stosb

        dec     rcx
        jz      .strn_cpy_ret  

        or      al, al
        jnz     .strn_cpy_loop

rep     stosb

.strn_cpy_ret:
        ret

; rdi <- dest
; rsi <- src

; rax -> dest

_arch_strcpy:

        cld
        mov     rax, rdi

.str_cpy_loop:

        lodsb
        stosb

        or      al, al
        jnz     .str_cpy_loop

        ret

; rdi <- dest
; rsi <- src
; rdx <- n

; rax -> dest

_arch_memcpy:

        pushfq
        push    rcx

        cld
        mov     rax, rdi
        mov     rcx, rdx
rep     movsb

        pop     rcx
        popfq
        ret

; rdi <- s
; rsi <- c
; rdx <- n

; rax -> s

_arch_memset:

        pushfq
        push    rcx
        push    rdi

        mov     rax, rsi
        and     rax, 0xff

        cld
        mov     rcx, rdx
rep     stosb
        
        pop     rdi
        mov     rax, rdi

        pop     rcx
        popfq
        ret
        
; rdi <- dest
; rsi <- src
; rdx <- n

; rax -> dest 

_arch_memmove:

        call _arch_memcpy
        ret

; rdi <- s1
; rsi <- s2
; rdx <- n

; rax -> 0 when s1 = s2, > 0 when s1 > s2, < 0 when s1 < s2

_arch_memcmp:

        pushfq
        push    rcx

        cld
        xor     rax, rax

        mov     rcx, rdx
repz    cmpsb

        jz      .mem_cmp_done
        
        mov     al, byte [rdi - 1]
        sub     al, byte [rsi - 1]

        cbw
        movsx   rax, ax

.mem_cmp_done:

        pop     rcx
        popfq
        ret