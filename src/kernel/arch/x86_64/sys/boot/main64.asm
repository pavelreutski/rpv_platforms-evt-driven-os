global _long_mode_start

extern _stack_top

extern _ps2_controller
extern _irqs_exceptions

extern _cstart_entry

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

    ; PS/2 controller
    call    _ps2_controller
    
    ; Hardware IRQs and CPU exceptions
    call    _irqs_exceptions

    mov     rsp, _stack_top
    sti
    
    jmp     _cstart_entry ; C kernel entry