global start

[section .text]

[bits 32]

start:
    ; print 'OK'
    mov     edi, 0xb8000
    mov     dword [edi], 0x2f4b2f4f ; OK with 2f char attr
    hlt