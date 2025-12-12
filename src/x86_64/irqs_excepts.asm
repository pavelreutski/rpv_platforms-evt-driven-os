global _irqs_exceptions

; LAPIC
extern _lapic_enable
extern _ioapic_redirect_irq

; ISRs
extern _isr_keyboard

IDT_ENTRIES         equ     256 ; ISR vectors

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

idt64:  times IDT_ENTRIES   resb    idt_entry_size

[section .rodata]

idtr:

        dw      (IDT_ENTRIES * idt_entry_size) - 1 ; limit
        dq      idt64                              ; base

[section .text]

[bits 64]

_irqs_exceptions:

        push    rdi
        push    rsi

        ; set IDT entries (vectors)

        mov     rdi, 0x21          
        mov     rsi, _isr_keyboard
        call    idt_set_gate ; set and enable 0x21 IDT entry (vector)

        ; init LAPIC
        
        call    _lapic_enable

        ; IRQs to vectors mapping (shift hardware IRQs > 31 since 0 - 31 vectors are CPU exceptions)

        mov     rdi, 1
        mov     rsi, 0x21
        call    _ioapic_redirect_irq ; remap IRQ1 (keyboard) -> 0x21 vector

        lidt    [idtr] ; set IDT64

        pop     rsi
        pop     rdi
        ret

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