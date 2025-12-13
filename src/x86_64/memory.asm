global _arch_memcpy, _arch_memset, _arch_memmove, _arch_memcmp

[section .text]

[bits 64]

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

; rax -> 0 when s1 = s2, 1 when s1 > s2, -1 when s1 < s2

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