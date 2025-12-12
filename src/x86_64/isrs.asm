global _isr_keyboard

extern _lapic_eoi
extern _kernel_onKeybScan

[section .text]

[bits 64]

_isr_keyboard:

    push    rbp
    mov     rbp, rsp

    push    rax
    push    rdi

    xor     rax, rax
    mov     rdi, rax

    in      al, 0x60

    ; filter possible IRQ leakage

    cmp     al, 0xAA ; keyb self-test passed 
    jz      .keyb_eoisr

    cmp     al, 0xfa ; keyb command ACK
    jz      .keyb_eoisr  

    mov     dil, al
    call    _kernel_onKeybScan ; call kernel keyboard routine

.keyb_eoisr:
    call    _lapic_eoi      ; signal end of interrupt

    pop     rdi
    pop     rax
    pop     rbp

    iretq