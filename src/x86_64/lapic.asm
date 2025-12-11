global  _lapic_eoi
global  _lapic_enable

SVR_REG             equ     0xF0
EOI_REG             equ     0xB0

LAPIC_BASE_ADDR     equ     0x40200000 ; virtual address

LAPIC_SVR           equ     (LAPIC_BASE_ADDR + SVR_REG)
LAPIC_EOI           equ     (LAPIC_BASE_ADDR + EOI_REG)

[section .text]

[bits 64]

_lapic_eoi:

        mov     dword [LAPIC_EOI], 0
        ret

_lapic_enable:

        push    rax
        push    rcx
        push    rdx

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

        mov     eax, [LAPIC_SVR]
        or      eax, 0x100
        mov     [LAPIC_SVR], eax

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