global _isr_keyboard
extern _kernel_keybIRQ

[section .text]

[bits 64]

_isr_keyboard:

    push    rbp
    mov     rbp, rsp

    push    rax
    call    _kernel_keybIRQ ; call kernel keyboard routine

    pop     rax
    pop     rbp

    iretq