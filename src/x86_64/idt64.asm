global idtr, idt_set_gate

IDT_ENTRIES         equ     256

FLAG_PRESENT        equ     0x80
FLAG_INT_GATE       equ     0x0E

struc idt_entry

    .offset_low     resw    1   ; dw
    .selector       resw    1   ; dw
    .ist            resb    1   ; db
    .type_attr      resb    1   ; db
    .offset_mid     resw    1   ; dw
    .offset_high    resd    1   ; dd
    .zero           resd    1   ; dd

endstruc

[section .bss]

idt64: times IDT_ENTRIES   resb    idt_entry_size

[section .rodata]

idtr:

    dw      (IDT_ENTRIES * idt_entry_size) - 1 ; limit
    dq      idt64                              ; base

[section .text]

[bits 64]

; rdi <- IRQ_N
; rsi <- isr address

idt_set_gate:

    push    rdi
    push    rax

    imul    rdi, idt_entry_size ; rdi <- address of IRQ_N id_entry
    add     rdi, idt64

    mov     rax, rsi
    and     rax, 0xFFFF

    mov     word [rdi + idt_entry.offset_low], ax
    mov     word [rdi + idt_entry.selector], 0x08

    mov     byte [rdi + idt_entry.ist], 0
    mov     byte [rdi + idt_entry.type_attr], (FLAG_PRESENT | FLAG_INT_GATE)

    mov     rax, rsi
    shr     rax, 16
    and     rax, 0xFFFF

    mov     word [rdi + idt_entry.offset_mid], ax

    mov     rax, rsi
    shr     rax, 32

    mov     dword [rdi + idt_entry.offset_high], eax
    mov     dword [rdi + idt_entry.zero], 0     

    pop     rax
    pop     rdi

    ret