global long_mode_start

[section .text]

[bits 64]

long_mode_start:

    xor     ax, ax ; load null into all data segment registers

    mov     ss, ax
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax

    ; print 'OK'
    mov     edi, 0xb8000
    mov     dword [edi], 0x2f4b2f4f ; OK with 2f char attr

    hlt