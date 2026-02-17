global _arch_hide_cursor, _arch_show_cursor, _arch_set_cursor_pos

VGA_TEXT_COLUMNS                equ     80

VGA_REG_DATA_PORT               equ     0x3D5
VGA_REG_ADDRESS_PORT            equ     0x3D4

; Cursor Start Register (0x0A)
; --------------------------------
; Bit 7-6: reserved
; Bit 5:   Cursor Disable (1 = hidden, 0 = visible)
; Bit 4-0: Starting scanline (0–15)

VGA_CURSOR_START_REG            equ     0x0A

VGA_CURSOR_VISIBLE_MASK         equ     0x20
VGA_CURSOR_START_SCANLINE_MASK  equ     0x1F


; Cursor End Register (0x0B)
; --------------------------------
; Bit 7-5: reserved
; Bit 4-0: Ending scanline (0–15)

VGA_CURSOR_END_REG              equ     0x0B

VGA_CURSOR_END_SCANLINE_MASK    equ     0x1F

VGA_CURSOR_DATA_HI_REG          equ     0x0E
VGA_CURSOR_DATA_LOW_REG         equ     0x0F

[section .text]

[bits 64]

_arch_show_cursor:

            mov     rdi, VGA_CURSOR_START_REG
            call    select_vga_reg            

            and     al, ~(VGA_CURSOR_VISIBLE_MASK)
            out     dx, al

            mov     rdi, VGA_CURSOR_END_REG
            call    select_vga_reg

            or      al, VGA_CURSOR_END_SCANLINE_MASK
            out     dx, al

            ret

_arch_hide_cursor:

            mov     rdi, VGA_CURSOR_START_REG
            call    select_vga_reg

            or      al, VGA_CURSOR_VISIBLE_MASK
            out     dx, al

            ret

; rdi <- row
; rsi <- column

_arch_set_cursor_pos:

            imul    rdi, VGA_TEXT_COLUMNS
            add     rdi, rsi

            mov     rcx, rdi

            mov     rdi, VGA_CURSOR_DATA_HI_REG
            call    select_vga_reg
            
            mov     rax, rcx            

            xchg    ah, al
            out     dx, al   ; <- write pos word hi

            mov     rdi, VGA_CURSOR_DATA_LOW_REG
            call    select_vga_reg

            mov     rax, rcx            
            out     dx, al    ; <- write pos word low

            ret

; rdi <- VGA register address
; rax -> VGA register data
; rdx -> VGA data register port

select_vga_reg:

            xor     rax, rax
            mov     al, dil

            mov     dx, VGA_REG_ADDRESS_PORT
            out     dx, al

            mov     dx, VGA_REG_DATA_PORT
            in      al, dx

            ret