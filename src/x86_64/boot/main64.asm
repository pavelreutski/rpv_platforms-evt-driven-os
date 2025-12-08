global _long_mode_start

extern idtr, stack_top
extern idt_set_gate

extern _isr_keyboard

extern _kernel_main

[section .text]

[bits 64]

_long_mode_start:

    xor     ax, ax ; load null into all data segment registers

    mov     ss, ax
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax

    cli

    mov     rdi, 0x21          
    mov     rsi, _isr_keyboard

    call    idt_set_gate ; remap IRQ1 -> 0x21 IDT entry (vector)

    lidt    [idtr]

    mov     rsp, stack_top
    jmp     _kernel_main