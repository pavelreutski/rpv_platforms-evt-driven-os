global _isr_keyboard

extern _lapic_eoi
extern _kernel_keybIRQ

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

    mov     dil, al
    call    _kernel_keybIRQ ; call kernel keyboard routine
    
    call    _lapic_eoi      ; signal end of interrupt

    pop     rdi
    pop     rax
    pop     rbp

    iretq