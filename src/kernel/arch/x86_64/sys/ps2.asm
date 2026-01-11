global _ps2_controller

; 0 	Output buffer status (0 = empty, 1 = full) (must be set before attempting to read data from IO port 0x60)
; 1 	Input buffer status (0 = empty, 1 = full) (must be clear before attempting to write data to IO port 0x60 or IO port 0x64)
; 2 	System Flag. Meant to be cleared on reset and set by firmware (via. PS/2 Controller Configuration Byte) if the system passes self tests (POST)
; 3 	Command/data (0 = data written to input buffer is data for PS/2 device, 1 = data written to input buffer is data for PS/2 controller command)
; 4 	Unknown (chipset specific). May be "keyboard lock" (more likely unused on modern systems)
; 5 	Unknown (chipset specific). May be "receive time-out" or "second PS/2 port output buffer full"
; 6 	Time-out error (0 = no error, 1 = time-out error)
; 7 	Parity error (0 = no error, 1 = parity error)

PS2_ST_OE_FLAG      equ     (1 << 0)
PS2_ST_IE_FLAG      equ     (1 << 1)
PS2_CMD_DATA_FLAG   equ     (1 << 3)

; 0x20 	        Read "byte 0" from internal RAM. Controller Configuration Byte (see below)
; 0x21 to 0x3F 	Read "byte N" from internal RAM (where 'N' is the command byte & 0x1F). Unknown (only the first byte of internal RAM has a standard purpose)
; 0x60 	        Write next byte to "byte 0" of internal RAM (Controller Configuration Byte, see below)
; 0x61 to 0x7F 	Write next byte to "byte N" of internal RAM (where 'N' is the command byte & 0x1F)
; 0xA7       	Disable second PS/2 port (only if 2 PS/2 ports supported)
; 0xA8      	Enable second PS/2 port (only if 2 PS/2 ports supported)

; 0xA9 	        Test second PS/2 port (only if 2 PS/2 ports supported). 0x00 test passed
;                                                                       0x01 clock line stuck low 
;                                                                       0x02 clock line stuck high 
;                                                                       0x03 data line stuck low 
;                                                                       0x04 data line stuck high

; 0xAA 	        Test PS/2 Controller. 0x55 test passed
;                                     0xFC test failed

; 0xAB          Test first PS/2 port. 0x00 test passed
;                                     0x01 clock line stuck low 
;                                     0x02 clock line stuck high 
;                                     0x03 data line stuck low 
;                                     0x04 data line stuck high

; 0xAC 	        Diagnostic dump (read all bytes of internal RAM)
; 0xAD 	        Disable first PS/2 port
; 0xAE 	        Enable first PS/2 port
; 0xC0 	        Read controller input port
; 0xC1 	        Copy bits 0 to 3 of input port to status bits 4 to 7
; 0xC2      	Copy bits 4 to 7 of input port to status bits 4 to 7
; 0xD0      	Read Controller Output Port. Controller Output Port (see below)
; 0xD1       	Write next byte to Controller Output Port (see below). Note: Check if output buffer is empty first
; 0xD2       	Write next byte to first PS/2 port output buffer (only if 2 PS/2 ports supported). (makes it look like the byte written was received from the first PS/2 port)
; 0xD3      	Write next byte to second PS/2 port output buffer (only if 2 PS/2 ports supported). (makes it look like the byte written was received from the second PS/2 port)
; 0xD4 	        Write next byte to second PS/2 port input buffer (only if 2 PS/2 ports supported). (sends next byte to the second PS/2 port)
; 0xF0 to 0xFF 	Pulse output line low for 6 ms. Bits 0 to 3 are used as a mask (0 = pulse line, 1 = don't pulse line) and correspond to 4 different output lines.
;                                               Note: Bit 0 corresponds to the "reset" line. The other output lines don't have a standard/defined purpose.

PS2_READ_CFG_BYTE       equ     0x20
PS2_WRITE_CFG_BYTE      equ     0x60

PS2_DISABLE_1ST_PORT    equ     0xAD
PS2_DISABLE_2ND_PORT    equ     0xA7
PS2_ENABLE_1ST_PORT     equ     0xAE
PS2_ENABLE_2ND_PORT     equ     0xA8

; 0 	First PS/2 port interrupt (1 = enabled, 0 = disabled)
; 1 	Second PS/2 port interrupt (1 = enabled, 0 = disabled, only if 2 PS/2 ports supported)
; 2 	System Flag (1 = system passed POST, 0 = your OS shouldn't be running)
; 3 	Should be zero
; 4 	First PS/2 port clock (1 = disabled, 0 = enabled)
; 5 	Second PS/2 port clock (1 = disabled, 0 = enabled, only if 2 PS/2 ports supported)
; 6 	First PS/2 port translation (1 = enabled, 0 = disabled)
; 7 	Must be zero

PS2_CFG_1ST_PORT_IRQ_ENABLE_FLAG        equ     (1 << 0)
PS2_CFG_2ND_PORT_IRQ_ENABLE_FLAG        equ     (1 << 1)
PS2_CFG_1ST_PORT_CLCK_DISABLE_FLAG      equ     (1 << 4)
PS2_CFG_2ND_PORT_CLCK_DISABLE_FLAG      equ     (1 << 5)
PS2_CFG_1ST_PORT_TRANSL_ENABLE_FLAG     equ     (1 << 6)

; PS/2 keyboard (device) commands

; 0xFF 	Reset and start self-test. 0xFA (ACK) or 0xFE (Resend) followed by 0xAA (self-test passed), 0xFC or 0xFD (self test failed) 
; 0xF5 	Disable scanning (keyboard won't send scan codes). Note: May also restore default parameters. 0xFA (ACK) or 0xFE (Resend)
; 0xF0 	Get/set current scan code set 	Sub-command: Value 	Use
;                                                     0 	Get current scan code set
;                                                     1 	Set scan code set 1
;                                                     2 	Set scan code set 2
;                                                     3 	Set scan code set 3
;                                       0xFA (ACK) or 0xFE (Resend) if scan code is being set; 0xFA (ACK) then the scan code set number, or 0xFE (Resend) if you're getting the scancode. 
;                                       If getting the scancode the table indicates the value that identify each set: Raw 	Translated 	Use
;                                                                                                                      1 	0x43 	    Scan code set 1
;                                                                                                                      2 	0x41 	    Scan code set 2
;                                                                                                                      3 	0x3F 	    Scan code set 3
; 0xF4 	Enable scanning (keyboard will send scan codes). 0xFA (ACK) or 0xFE (Resend) 

PS2_KEYB_RST_SELFTEST       equ     0xFF
PS2_KEYB_SCAN_CODESET       equ     0xF0
PS2_KEYB_SCAN_ENABLE        equ     0xF4
PS2_KEYB_SCAN_DISABLE       equ     0xF5

; PS/2 IO ports

PS2_DATA_PORT               equ     0x60
PS2_CMD_ST_PORT             equ     0x64

[section .text]

[bits 64]

_ps2_controller:
        push    rax

        ; check if ps/2 present
        call ps2_present

        ; reset ps/2 controller

        ; disable ps/2 first port
        mov     al, PS2_DISABLE_1ST_PORT
        call    send_ps2_command

        ; disable ps/2 second port
        mov     al, PS2_DISABLE_2ND_PORT
        call    send_ps2_command

        ; flush ps/2 output buffer
        call    flush_ps2_data

        ; read ps/2 config byte
        mov     al, PS2_READ_CFG_BYTE
        call    send_ps2_command
        call    recv_ps2_data

        mov     ah, al

        ; enable IRQ1. disable XT translation <- scan code set 2 is expected

        or      ah, PS2_CFG_1ST_PORT_IRQ_ENABLE_FLAG
        and     ah, ~(PS2_CFG_1ST_PORT_TRANSL_ENABLE_FLAG)

        mov     al, PS2_WRITE_CFG_BYTE
        call    send_ps2_command

        mov     al, ah
        call    send_ps2_data

        ; enable ps/2 first port
        mov     al, PS2_ENABLE_1ST_PORT
        call    send_ps2_command

        ; reset ps/2 keyboard       

        ; start reset self-test
        mov     al, PS2_KEYB_RST_SELFTEST
        call    send_ps2_data

        call    recv_ps2_data ; 0xFA ACK
        call    recv_ps2_data ; 0xAA self-test pass

        ; disable scanning
        mov     al, PS2_KEYB_SCAN_DISABLE
        call    send_ps2_data
        call    recv_ps2_data ; 0xFA ACK

        ; set scan code set
        mov     al, PS2_KEYB_SCAN_CODESET
        call    send_ps2_data

        call    recv_ps2_data ; 0xFA ACK

        mov     al, 2         ; choose set 2
        call    send_ps2_data

        call    recv_ps2_data ; 0xFA ACK

        ; enable scanning
        mov     al, PS2_KEYB_SCAN_ENABLE
        call    send_ps2_data

        call    recv_ps2_data ; 0xFA ACK

        pop     rax
        ret

; rax <- 1 (true) - it exists. 0 (false) - it doesnt exist
; todo: use acpi instead of try read ps2 ports. now when ps2 doesnt exist it will loop forever here
ps2_present:

        xor     rax, rax

        mov     al, PS2_READ_CFG_BYTE
        call    send_ps2_command

        call    recv_ps2_data

        mov     rax, 1
        ret

; al <- command
send_ps2_command:
        call    wait_ps2_ie
        out     PS2_CMD_ST_PORT, al
        ret

; al <- send data
send_ps2_data:
        call    wait_ps2_ie
        out     PS2_DATA_PORT, al 
        ret

; al <- recv data
recv_ps2_data:
        call    wait_ps2_noe
        in      al, PS2_DATA_PORT
        ret

flush_ps2_data:
        push    rax

.ps2_data_flush_loop:
        in      al, PS2_CMD_ST_PORT

        test    al, PS2_ST_OE_FLAG
        jz      .done_ps2_data_flush

        in      al, PS2_CMD_DATA_FLAG

        jmp     .ps2_data_flush_loop

.done_ps2_data_flush:
        pop     rax       
        ret

wait_ps2_ie:
        push    rax
.wait_ie_loop:

        in      al, PS2_CMD_ST_PORT
        test    al, PS2_ST_IE_FLAG
        jnz     .wait_ie_loop      ; is input buffer full ?

        pop     rax
        ret

wait_ps2_noe:
        push    rax
.wait_oe_loop:

        in      al, PS2_CMD_ST_PORT
        test    al, PS2_ST_OE_FLAG
        jz      .wait_oe_loop       ; is output buffer has a byte to read ?

        pop     rax
        ret