global  lapic_enable

SVR_REG             equ     0xF0
LAPIC_ADDRESS       equ     0x40200000

[section .text]

[bits 64]

lapic_enable:
        push    rax
        push    rcx
        push    rdx

        push    rsi

        ; disable legacy PIC master/slave

        call    pic_disable

        ; enable lapic

        mov     ecx, 0x1B
        rdmsr                  ; edx:eax <- msr value

        shl     rdx, 32
        or      rax, rdx

        or      rax, (1 << 11) ; lapic enable flag

        mov     rdx, rax
        shr     rdx, 32        ; high
        wrmsr

        ; enable lapic spurious irqs

        mov     rsi, LAPIC_ADDRESS
        mov     eax, [rsi + SVR_REG]

        or      eax, 0x100
        mov     [rsi + SVR_REG], eax

        pop     rsi

        pop     rdx
        pop     rcx
        pop     rax

        ret

pic_disable:

        push    ax

        ; mask all IRQs

        mov     al, 0xFF
        out     0x21, al
        out     0xA1, al

        ; ICW1 : start init sequence

        mov     al, 0x11
        out     0x20, al
        out     0xA0, al

        ; ICW2 : interrupt vector offsets

        mov     al, 0xF0
        out     0x21, al
        mov     al, 0xF8
        out     0xA1, al

        ; ICW3 : wiring information

        mov     al, 4
        out     0x21, al
        mov     al, 2
        out     0xA1, al

        ; ICW3: 8086 mode

        mov     al, 1
        out     0x21, al
        out     0xA1, al

        ; mask all IRQs again

        mov     al, 0xFF
        out     0x21, al
        out     0xA1, al

        pop     ax
        ret