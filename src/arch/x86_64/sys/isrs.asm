global _isr_keyboard

extern _lapic_eoi
extern _kernel_onKeybScan

[section .text]

[bits 64]

_isr_keyboard:

    push    rax
    push    rcx
    push    rdx
    
    push    rsi
    push    rdi

    push    r8
    push    r9
    push    r10
    push    r11

    in      al, 0x60

    ; filter possible IRQ leakage

    cmp     al, 0xAA ; keyb self-test passed 
    jz      .keyb_eoisr

    cmp     al, 0xfa ; keyb command ACK
    jz      .keyb_eoisr

    sub     rsp, 8

    movzx   edi, al
    call    _kernel_onKeybScan ; call kernel keyboard routine

    add     rsp, 8

.keyb_eoisr:
    call    _lapic_eoi      ; signal end of interrupt

    pop     r11
    pop     r10
    pop     r9
    pop     r8

    pop     rdi
    pop     rsi

    pop     rdx
    pop     rcx
    pop     rax

    iretq