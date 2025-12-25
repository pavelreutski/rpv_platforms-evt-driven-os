global  _lapic_eoi
global  _lapic_enable

global  _lapic_timer
global  _lapic_timer_tcv
global  _lapic_timer_max_cv
global  _lapic_timer_ms_ticks

; ---------------------------- PIT ----------------------------------------

PIT_CHANNEL_PORT        equ     0x40
PIT_COMMAND_PORT        equ     0x43

PIT_CH_0_SEL            equ     0
PIT_CH_0_LATCH          equ     0
PIT_CH_BINARY_CNT       equ     0
PIT_CH_MODE_4           equ     (4 << 1)
PIT_CH_LO_HI_ORDER      equ     (3 << 4)

;00 11 100 0: ch0, lobyte/hibyte, mode 4, binary

PIT_10MS_TICKS          equ     0x2E9B   ; ticks for ~10ms
PIT_INPUT_FREQ          equ     0x1234DE ; 1193182 Hz

;  --------------------------- LAPIC --------------------------------------

LAPIC_BASE_ADDR         equ     0x40200000 ; virtual address

EOI_REG                 equ     0xB0
SVR_REG                 equ     0xF0

LVT_TIMER_REG           equ     0x320 ; Timer mode, vector, mask/unmask
TMICT_REG               equ     0x380 ; Initial counter value for countdown
TMCCT_REG               equ     0x390 ; Current countdown value
DFR_REG                 equ     0x3E0 ; Clock divider for timer input

LAPIC_SVR_ADDR          equ     (LAPIC_BASE_ADDR + SVR_REG)
LAPIC_EOI_ADDR          equ     (LAPIC_BASE_ADDR + EOI_REG)
LAPIC_DFR_ADDR          equ     (LAPIC_BASE_ADDR + DFR_REG)
LAPIC_TMICT_ADDR        equ     (LAPIC_BASE_ADDR + TMICT_REG)
LAPIC_TMCCT_ADDR        equ     (LAPIC_BASE_ADDR + TMCCT_REG)
LAPIC_LVT_TMR_ADDR      equ     (LAPIC_BASE_ADDR + LVT_TIMER_REG)

LAPIC_TIMER_DIV_16      equ     0x03  ; Input freq divide by 16

LAPIC_TIMER_IRQ0_SEL    equ     0     ; IRQ0 select
LAPIC_TIMER_ONE_SHOT    equ     0     ; One shot event

LAPIC_TIMER_IRQ_MASK    equ     (1 << 16) ; disable IRQ
LAPIC_TIMER_PERIODIC    equ     (1 << 17) ; periodic events

[section .bss]

lapic_timer_freq_hz     dq      ?
lapic_timer_divider     dd      ?

[section .text]

[bits 64]

_lapic_eoi:

                mov     dword [LAPIC_EOI_ADDR], 0
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

                mov     eax, [LAPIC_SVR_ADDR]
                or      eax, 0x100
                mov     [LAPIC_SVR_ADDR], eax

                pop     rdx
                pop     rcx
                pop     rax

                ret

; rdi <- milliseconds
; rax -> timer ticks for given milliseconds

_lapic_timer_ms_ticks:                

                mov     rax, [lapic_timer_freq_hz]                
                mul     rdi
                mov     rdi, 1000
                div     rdi                                         
                ret

; rax -> max timer ticks

_lapic_timer_max_cv:
                mov     eax, [LAPIC_TMICT_ADDR]
                ret

; rax -> timer ticks

_lapic_timer_tcv:                         
                mov     eax, [LAPIC_TMICT_ADDR]
                sub     eax, [LAPIC_TMCCT_ADDR]
                ret

; lapic system timer

_lapic_timer:
                push    rax
                push    rbx
                push    rcx
                push    rdx

                xor     rax, rax

                mov     [lapic_timer_freq_hz], rax
                mov     [lapic_timer_divider], eax
                               
                mov     dword [LAPIC_DFR_ADDR], LAPIC_TIMER_DIV_16 ; set lapic timer divider                
                mov     dword [LAPIC_LVT_TMR_ADDR], (LAPIC_TIMER_IRQ_MASK | LAPIC_TIMER_PERIODIC) ; mask timer IRQ | periodic events (free running)

                ; Guess timer frequency

                ; Write initial count                
                mov     dword [LAPIC_TMICT_ADDR], 0xFFFFFFFF

                ; Wait ~10ms
                call    pit_wait_10ms

                ; Read current count
                xor     rax, rax
                mov     eax, [LAPIC_TMCCT_ADDR]

                ; Calculate ticks per second
                ; ticks_elapsed = 0xFFFFFFFF - current_count

                mov     rbx, 0xFFFFFFFF
                sub     rbx, rax

                ; timer_frequency_hz = ticks_elapsed * 100                

                mov     rax, rbx
                mov     rcx, 100
                mul     rcx                  ; rdx:rax = rax * 100

                mov     [lapic_timer_freq_hz], rax ; store 64 bit result
                mov     dword [lapic_timer_divider], LAPIC_TIMER_DIV_16

                pop rdx
                pop rcx
                pop rbx
                pop rax

                ret

pit_wait_10ms:

                push rax           

                ; set PIT channel 0, mode 4 (software triggered strobe), lobyte/hibyte

                mov al, (PIT_CH_MODE_4 | PIT_CH_LO_HI_ORDER)
                out PIT_COMMAND_PORT, al

                ; Send count value (10 ms ~ 11931 or 0x2E9b)

                mov     ax, PIT_10MS_TICKS

                out     PIT_CHANNEL_PORT, al ; low byte
                shr     ax, 8
                out     PIT_CHANNEL_PORT, al ; high byte

        .pit_wait_loop:

                xor     al, al
                out     PIT_COMMAND_PORT, al ; latch counter reg

                in      al, PIT_CHANNEL_PORT ; low byte
                shl     ax, 8               
                in      al, PIT_CHANNEL_PORT ; high byte                

                or      ax, ax
                jnz     .pit_wait_loop            

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