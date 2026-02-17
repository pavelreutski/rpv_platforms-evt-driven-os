global _ioapic_redirect_irq

IOAPIC_ADDRESS      equ     0x40000000

IOAPIC_REG_SEL      equ     0
IOAPIC_WINDOW       equ     0x10

[section .text]

[bits 64]

; rdi <- irq_n
; rsi <- idt gate vector number

_ioapic_redirect_irq:

        push    rax
        push    rbx
        push    rcx

        push    rdi

        mov     rbx, rdi
        shl     rbx, 1            ; irq*2 for register index 

        mov     eax, IOAPIC_WINDOW
        add     eax, ebx          ; eax = IOAPIC lower register index
        mov     ecx, eax          ; select low register
        mov     eax, esi          ; vector (bits 0-7)

        mov     rdi, IOAPIC_ADDRESS

        mov     [rdi + IOAPIC_REG_SEL], ecx
        mov     [rdi + IOAPIC_WINDOW], eax

        xor     eax, eax          ; CPU0 always
        inc     ecx               ; select high register

        mov     [rdi + IOAPIC_REG_SEL], ecx
        mov     [rdi + IOAPIC_WINDOW], eax


        pop     rdi

        pop     rcx
        pop     rbx
        pop     rax

        ret