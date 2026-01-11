global _arch_memcpy, _arch_memset, _arch_memmove, _arch_memcmp
global _arch_strcpy, _arch_strcat, _arch_strncpy, _arch_strcmp, _arch_strncmp, _arch_strchr, _arch_strlen

[section .text]

[bits 64]

; rdi <- dest
; rsi <- src

; rax -> dest

_arch_strcat:
                push    rbx

                cld
                mov     rbx, rdi
                
                xor     rax, rax

                mov     rcx, -1                
        repnz   scasb

                dec     rdi
.strcat_loop:

                lodsb
                stosb

                or      al, al
                jnz     .strcat_loop 
               
                mov     rax, rbx

                pop     rbx
                ret

; rdi <- string
; rax -> length

_arch_strlen:
                cld
                xor     rcx, rcx
                mov     rsi, rdi

.strlen_loop:
                lodsb
                inc     rcx

                or      al, al
                jnz     .strlen_loop

                dec     rcx
                mov     rax, rcx
                ret

; rdi <- string
; rsi <- character

; rax -> string ptr when char in string or 0 otherwise

_arch_strchr:        
                mov     dl, sil

.strchr_loop:
                mov     al, [rdi]

                cmp     al, dl
                jz      .strchr_matched

                or      al, al
                jz      .strchr_not_matched

                inc     rdi
                jmp     .strchr_loop

.strchr_not_matched:
                xor     rax, rax
                ret

.strchr_matched:
                mov     rax, rdi
                ret

; rdi <- s1
; rsi <- s2

; rax -> 0 when s1 = s2, > 0 when s1 > s2, < 0 when s1 < s2

_arch_strcmp:
                mov     al, [rdi]
                mov     cl, [rsi]

                cmp     al, cl
                jnz     .strcmp_not_matched

                or      al, al
                jz      .strcmp_matched

                inc     rdi
                inc     rsi
                jmp     _arch_strcmp

.strcmp_not_matched:

                movzx   rax, al
                movzx   rcx, cl

                sub     rax, rcx
                ret

.strcmp_matched:

                xor     rax, rax
                ret

; rdi <- s1
; rsi <- s2
; rdx <- n

; rax -> 0 when s1 = s2, > 0 when s1 > s2, < 0 when s1 < s2

_arch_strncmp:
                mov     rcx, rdx

                or      rcx, rcx
                jz      .strncmp_matched

.strncmp_loop:
                mov     al, [rdi]
                mov     dl, [rsi]

                cmp     al, dl
                jnz     .strncmp_not_matched

                or      al, al
                jz      .strncmp_matched

                inc     rdi
                inc     rsi

                dec     rcx                
                jnz     .strncmp_loop                

.strncmp_not_matched:

                movzx   rax, al
                movzx   rdx, dl

                sub     rax, rdx
                ret

.strncmp_matched:
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