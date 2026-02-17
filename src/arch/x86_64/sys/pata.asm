global _arch_ide_device_detect, _arch_ide_device_read, _arch_ide_device_write, _arch_ide_device_flush

extern _lapic_timer_tcv
extern _lapic_timer_max_cv
extern _lapic_timer_ms_ticks

; | Channel       | Base (Data) | Control | IRQ    |
; | ------------- | ----------- | ------- | ------ |
; | **Primary**   | **`0x1F0`** | `0x3F6` | IRQ 14 |
; | **Secondary** | **`0x170`** | `0x376` | IRQ 15 |

; | Offset |        I/O Port | Register Name    | R/W | Purpose                    |
; | -----: | --------------: | ---------------- | --- | -------------------------- |
; |   `+0` | `0x1F0 / 0x170` | DATA             | R/W | Data (16-bit words)        |
; |   `+1` |            `+1` | ERROR / FEATURES | R   | Error flags                |
; |   `+2` |            `+2` | SECTOR COUNT     | W   | Sector count               |
; |   `+3` |            `+3` | LBA LOW          | W   | LBA bits 0–7               |
; |   `+4` |            `+4` | LBA MID          | W/R | LBA bits 8–15 / signature  |
; |   `+5` |            `+5` | LBA HIGH         | W/R | LBA bits 16–23 / signature |
; |   `+6` |            `+6` | DRIVE / HEAD     | W   | Drive select               |
; |   `+7` |            `+7` | STATUS / COMMAND | R/W | Status flags / command     |

PRIMARY_IDE_DATA_REG_PORT       equ         0x1F0
PRIMARY_IDE_CONTROL_REG_PORT    equ         0x3F6

SECONDARY_IDE_DATA_REG_PORT     equ         0x170
SECONDARY_IDE_CONTROL_REG_PORT  equ         0x376

; | Bit | Name     | Meaning                                            |
; | --- | -------- | -------------------------------------------------- |
; | 7   | —        | Reserved                                           |
; | 6   | —        | Reserved                                           |
; | 5   | —        | Reserved                                           |
; | 4   | —        | Reserved                                           |
; | 3   | —        | Reserved                                           |
; | 2   | **SRST** | Software reset (1 = reset device)                  |
; | 1   | **nIEN** | Interrupt enable (0 = enable IRQ, 1 = disable IRQ) |
; | 0   | —        | Reserved                                           |

IDE_CONTROL_SW_RESET            equ         (1 << 2)
IDE_CONTROL_DISABLE_IRQ         equ         (1 << 1)

; | Bit | Name  | Meaning                |
; | --: | ----- | ---------------------- |
; |   7 | BBK   | Bad block              |
; |   6 | UNC   | Uncorrectable data     |
; |   5 | MC    | Media changed          |
; |   4 | IDNF  | ID not found           |
; |   3 | MCR   | Media change requested |
; |   2 | ABRT  | Command aborted        |
; |   1 | TK0NF | Track 0 not found      |
; |   0 | AMNF  | Address mark not found |

IDE_ERROR_FEAT_REG              equ         1
IDE_SECTOR_COUNT_REG            equ         2

IDE_LBA_LOW_REG                 equ         3
IDE_LBA_MID_REG                 equ         4
IDE_LBA_HIGH_REG                equ         5

; | Bit | Name | Meaning               |
; | --: | ---- | --------------------- |
; |   7 | 1    | Must be set           |
; |   6 | LBA  | LBA mode (1 = LBA)    |
; |   5 | 1    | Must be set           |
; |   4 | DEV  | 0 = Master, 1 = Slave |
; | 3–0 | HEAD | Head number (legacy)  |

IDE_DRIVE_HEAD_SLAVE            equ         (1 << 4)
IDE_DRIVE_HEAD_MAGIC            equ         (1 << 7) | (1 << 5)
IDE_DRIVE_HEAD_LBA_MODE         equ         (IDE_DRIVE_HEAD_MAGIC) | (1 << 6)

IDE_DRIVE_HEAD_REG              equ         6

; | Bit | Name | Meaning            |
; | --: | ---- | ------------------ |
; |   7 | BSY  | Device busy        |
; |   6 | DRDY | Device ready       |
; |   5 | DF   | Device fault       |
; |   4 | DSC  | Seek complete      |
; |   3 | DRQ  | Data request ready |
; |   2 | CORR | Corrected data     |
; |   1 | IDX  | Index              |
; |   0 | ERR  | Error occurred     |

; | Command         | Value  | Meaning               |
; | --------------- | ------ | --------------------- |
; | IDENTIFY        | `0xEC` | Identify ATA device   |
; | IDENTIFY PACKET | `0xA1` | Identify ATAPI device |
; | READ            | `0x20` | Read ATA device sectrs|

IDE_DEVICE_BUSY_FLAG            equ         (1 << 7)
IDE_DEVICE_READY_FLAG           equ         (1 << 6)
IDE_DEVICE_DRQ_FLAG             equ         (1 << 3)
IDE_DEVICE_ERROR_FLAG           equ         (1 << 0) | (1 << 5)

IDE_DEVICE_IDENTIFY_CMD         equ         0xEC
IDE_DEVICE_READ_SECT_CMD        equ         0x20
IDE_DEVICE_WRITE_SECT_CMD       equ         0x30
IDE_DEVICE_FLUSH_CACHE_CMD      equ         0xE7        

IDE_STATUS_COMMAND_REG          equ         7   ; Status - read, Command - write

IDE_NO_DEVICE                   equ         0   ; No ATA device present code
IDE_ATA_DEVICE                  equ         1   ; ATA device present code
IDE_ATAPI_DEVICE                equ         2   ; ATAPI device present code

IDE_IO_TIMEOUT_MS               equ         50  ; The timeout for any IDE I/O await in milliseconds

[section .text]

[bits 64]

; rdi <- ide bus (0 - primary, 1 - secondary)
; rsi <- device  (0 - master,  1 - slave)

; rdx <- ide device identity block buffer (256 words)

; rax -> 0 (no device), 1 (ATA device), 2 (ATAPI device)

_arch_ide_device_detect:

                push    r12            
                mov     r12, rdx ; save buffer pointer

                ; set ide bus base address

                call    select_ide_controller ; select ide controller, dx <- data port

                mov     al, sil      ; device number
                call    ide_identify ; device is selected

                or      al, al       ; no device on ide bus
                jz      .ide_device_detect_done     

                cmp     al, IDE_ATAPI_DEVICE
                jz      .ide_device_detect_done ; ATAPI has no identity data block

                ; read ide device identity block

                mov     rdi, r12                ; buffer pointer
                mov     rcx, 256

        rep     insw

        .ide_device_detect_done:

                movzx   rax, al
                
                pop     r12
                ret

; rdi <- ide bus (0 - primary, 1 - secondary)
; rsi <- device  (0 - master,  1 - slave)

; rax -> i/o status: 0 (error), 1 (success)

_arch_ide_device_flush:

                call    select_ide_controller ; dx <- data port

                xor     rdi, rdi
                mov     al, IDE_DRIVE_HEAD_MAGIC

                or      rsi, rsi
                jz      .ide_flush_device_selected ; its master

                or      al, IDE_DRIVE_HEAD_SLAVE   ; select slave

        .ide_flush_device_selected:
                
                mov     di, IDE_DRIVE_HEAD_REG

                call    write_ide_reg            ; select drive master / slave                
                call    ide_ata_spec_delay       ; ata spec delay (supposed ~400ns)                

                call    ide_device_disable_irq   ; disable device irq after when it is selected
                call    wait_ide_device_ready

                or      al, al
                jz      .ide_device_flush_done

                ; flush write cache

                mov     al, IDE_DEVICE_FLUSH_CACHE_CMD
                mov     di, IDE_STATUS_COMMAND_REG

                call    write_ide_reg                

                call    wait_ide_device_not_busy

        .ide_device_flush_done:

                movzx   rax, al
                ret

; rdi <- ide bus (0 - primary, 1 - secondary)
; rsi <- device  (0 - master,  1 - slave)

; rdx <- read data buffer

; rcx <- start LBA sector
; r8  <- number of sectors to read

; rax -> i/o status: 0 (error), 1 (success)

_arch_ide_device_read:

                push    r12
                mov     r12, rdx ; save buffer pointer

                xor     rax, rax

                call    ide_device_sectors_io ; program ide bus device for sectors I/O (in PIO mode), dx <- data port

                or      al, al
                jz      .ide_device_read_done ; ide bus device is not ready

                ; send read sectors command

                mov     al, IDE_DEVICE_READ_SECT_CMD
                mov     di, IDE_STATUS_COMMAND_REG

                call    write_ide_reg      ; send read sectors command                

                ; wait drq
                call    wait_ide_device_drq  ; BSY =0, DRQ = 1, ERR = 0

                or      al, al
                jz      .ide_device_read_done ; device not drq by timeout or error

                ; read sectors loop

                mov     rcx, r8

                or      rcx, rcx
                jnz     .ide_device_read_nxt_sect

                mov     rcx, 256

        .ide_device_read_nxt_sect:

                push    rcx

                call    wait_ide_device_not_busy

                or      al, al
                jz      .ide_device_read_done                              

                ; read sector 256 words = 512 bytes

                cld            
                                
                mov     rcx, 256 ; 256 words
                mov     rdi, r12 ; buffer pointer
        rep     insw

                call    ide_ata_spec_delay

                add     r12, 512 ; move pointer to next sector data

                pop     rcx
                loop    .ide_device_read_nxt_sect

                mov     al, 1

        .ide_device_read_done:

                movzx   rax, al

                pop     r12
                ret

; rdi <- ide bus (0 - primary, 1 - secondary)
; rsi <- device  (0 - master,  1 - slave)

; rdx <- write data buffer

; rcx <- start LBA sector
; r8  <- number of sectors to write

; rax -> i/o status: 0 (error), 1 (success)

_arch_ide_device_write:

                push    r12
                mov     r12, rdx

                xor     rax, rax

                call    ide_device_sectors_io ; program ide bus device for sectors I/O (in PIO mode), dx <- data port

                or      al, al
                jz      .ide_device_write_done

                mov     al, IDE_DEVICE_WRITE_SECT_CMD
                mov     di, IDE_STATUS_COMMAND_REG

                call    write_ide_reg ; send write sectors command

                ; wait drq
                call    wait_ide_device_drq      ; DRQ = 1, ERR = 0

                or      al, al
                jz      .ide_device_write_done

                mov     rcx, r8

                or      rcx, rcx
                jnz     .ide_device_write_nxt_sect

                mov     rcx, 256

        .ide_device_write_nxt_sect:

                push    rcx

                call    wait_ide_device_not_busy ; BSY = 0, ERR = 0

                or      al, al
                jz      .ide_device_write_done

                cld               

                mov     rcx, 256
                mov     rsi, r12

        .ide_device_write_nxt_word:

                outsw                
                loop    .ide_device_write_nxt_word

                call    ide_ata_spec_delay                

                add     r12, 512                                                          

                pop     rcx
                loop    .ide_device_write_nxt_sect

                call    wait_ide_device_not_busy                    

        .ide_device_write_done:

                movzx   rax, al

                pop     r12
                ret

; rdi <- ide bus (0 - primary, 1 - secondary)
; rsi <- device  (0 - master,  1 - slave)

; rcx <- start LBA sector
; r8  <- number of sectors

; rdx -> ide controller device data port (primary / secondary)
; al  -> 1 - device ready, 0 - device not ready or timeout

ide_device_sectors_io:

                ; set ide bus base port
                call    select_ide_controller ; select ide controller. dx <- data port                                

                ; select drive: primary -> master/slave, secondary -> master/slave in lba28 mode

                mov     rax, rcx ; lba start sector
                shr     rax, 24  ; top 4 bits

                or      rsi, rsi
                jz      .ide_io_device_selected   ; it is master

                or      rax, IDE_DRIVE_HEAD_SLAVE ; select slave

        .ide_io_device_selected:

                or      rax, IDE_DRIVE_HEAD_LBA_MODE
                mov     di, IDE_DRIVE_HEAD_REG

                call    write_ide_reg          ; select drive in LBA28 mode

                call    ide_ata_spec_delay     ; ata spec delay (supposed ~400ns)
                call    ide_device_disable_irq ; disable device IRQ after when it selected

                ; set count and LBA start sector low/mid/high bytes

                mov     rax, r8
                and     rax, 0xFF 
                mov     di, IDE_SECTOR_COUNT_REG

                call    write_ide_reg      ; sectors count

                mov     rax, rcx            
                mov     di, IDE_LBA_LOW_REG

                call    write_ide_reg      ; lba low

                mov     rax, rcx
                shr     rax, 8
                mov     di, IDE_LBA_MID_REG

                call    write_ide_reg      ; lba mid

                mov     rax, rcx
                shr     rax, 16
                mov     di, IDE_LBA_HIGH_REG

                call    write_ide_reg      ; lba high

                mov     rax, 1

                ret

; dx <- base port
; al <- drive number: master (0) / slave (1)

; al -> 0 - no device, 1 - ATA device, 2 - ATAPI device

ide_identify:            
            
                xor     rdi, rdi

                shl     al, 4
                or      al, IDE_DRIVE_HEAD_MAGIC

                mov     dil, IDE_DRIVE_HEAD_REG
                call    write_ide_reg            ; select drive master / slave
                
                call    ide_ata_spec_delay           
                
                mov     dil, IDE_SECTOR_COUNT_REG
                call    clear_ide_reg            ; clear sector count register
                
                mov     dil, IDE_LBA_LOW_REG
                call    clear_ide_reg            ; clear LBA low register
                
                mov     dil, IDE_LBA_MID_REG
                call    clear_ide_reg            ; clear LBA mid register
                
                mov     dil, IDE_LBA_HIGH_REG
                call    clear_ide_reg            ; clear LBA high register
                
                mov     al, IDE_DEVICE_IDENTIFY_CMD
                mov     dil, IDE_STATUS_COMMAND_REG
                call    write_ide_reg            ; send identify
                
                call    ide_ata_spec_delay
                
                call    read_ide_status_reg      ; read status            

                cmp     al, 0xFF
                jz      .device_not_detected

                or      al, al
                jnz     .device_detected

        .device_not_detected:
                
                mov     ax, IDE_NO_DEVICE
                ret

        .device_detected:

                ; check if its ATA or ATAPI device ?
                
                call    wait_ide_device_ready

                cmp     al, 1                   ; is device ready ?
                jz      .device_ready

                mov     ax, IDE_NO_DEVICE
                ret

        .device_ready:
            
                mov     dil, IDE_LBA_MID_REG
                call    read_ide_reg              ; LBA mid         

                xchg    ah, al
                
                mov     dil, IDE_LBA_HIGH_REG     ; LBA hi
                call    read_ide_reg            

                xchg    ah, al

                cmp     ax, 0xEB14
                jnz     .no_atapi_device

                mov     ax, IDE_ATAPI_DEVICE
                ret

        .no_atapi_device:            

                ; ATA device - check its status            
                call    read_ide_status_reg

                test    al, IDE_DEVICE_ERROR_FLAG
                jz      .no_ata_device_error

                mov     ax, IDE_NO_DEVICE
                ret

        .no_ata_device_error:

                call    wait_ide_device_drq

                ; when drq awaited al -> 1 (ATA device), when not awaited al -> 0 (no device)
                xor     ah, ah ; clear ah so ret value is consistent          
                ret

; rdi <- ide bus (0 - primary, 1 - secondary)
; rdx -> ide controller device data port (primary / secondary)

select_ide_controller:

                mov     rdx, SECONDARY_IDE_DATA_REG_PORT

                or      rdi, rdi
                jnz     .ide_bus_port_selected

                add     rdx, 0x80

        .ide_bus_port_selected:
                ret

; dx <- base port
; di <- ide register

clear_ide_reg:

                xor     al, al
                call    write_ide_reg

                ret

; dx <- base port
; di <- ide register
; al <- register value

write_ide_reg:

                push    rdx

                add     dx, di
                out     dx, al

                pop     rdx
                ret

; dx <- base port
; di <- ide register

; al -> register value

read_ide_reg:

                push    rdx

                add     dx, di
                in      al, dx

                pop     rdx
                ret

; dx <- base port
; di <- ide register

; al -> status register

read_ide_status_reg:

                mov     dil, IDE_STATUS_COMMAND_REG
                call    read_ide_reg

                ret

; dx <- base port

ide_ata_spec_delay:

                push    rax
                push    rcx
                push    rdx
                
                add     dx, 0x206
                mov     rcx, 4
        .ide_delay_loop:

                in      al, dx
                loop    .ide_delay_loop

                pop     rdx
                pop     rcx
                pop     rax
                ret

; dx <- base port

ide_device_disable_irq:

                push    rax
                push    rdx

                add     dx, 0x206
                mov     al, IDE_CONTROL_DISABLE_IRQ
                out     dx, al

                pop     rdx
                pop     rax
                ret

; rdi <- start ticks
; rsi <- timeout ticks
; rax -> 1 - wait is in progress, 0 - wait is elapsed

wait_ide_io_elapsed:

                push    rbx

                call    _lapic_timer_tcv    ; rax <- lapic current ticks

                mov     rbx, rax
                cmp     rdi, rbx
                jb      .elapsed_ticks_not_overflow

                call    _lapic_timer_max_cv ; rax <- lapic max ticks

                sub     rax, rdi
                add     rbx, rax
                
                jmp     .elapsed_timeout

        .elapsed_ticks_not_overflow:

                sub     rbx, rdi

        .elapsed_timeout:

                mov     rax, 1

                cmp     rbx, rsi
                jb      .elapsed_done

                xor     rax, rax
        .elapsed_done:
                pop     rbx
                ret

; dx <- base port
; al -> 1 - device is not busy, 0 - device error detected or timeout

wait_ide_device_not_busy:

                push    rdx
                push    rbx

                mov     bl, 1
                add     dx, IDE_STATUS_COMMAND_REG

        .wait_not_busy_loop:

                in      al, dx

                test    al, IDE_DEVICE_BUSY_FLAG    ; BSY = 0
                jnz     .wait_not_busy_loop             

                test    al, IDE_DEVICE_ERROR_FLAG   ; if ERR = 0
                jz      .wait_device_not_busy_done

                xor     bl, bl                      ; ERR = 1

        .wait_device_not_busy_done:

                mov     al, bl               

                pop     rbx
                pop     rdx
                ret

; dx <- base port
; al -> 1 - device ready, 0 - device not ready or timeout

wait_ide_device_ready:

                push    rdx
                push    rcx
                push    rbx

                mov     bl, 1

                add     dx, IDE_STATUS_COMMAND_REG

                mov     rcx, 0xFFFFFF

        .wait_ready_loop:

                in      al, dx

                test    al, IDE_DEVICE_BUSY_FLAG
                jz      .wait_device_not_busy

                loop    .wait_ready_loop
                jmp     .wait_ready_err_loop     

        .wait_device_not_busy:                

                test    al, IDE_DEVICE_READY_FLAG
                jnz     .wait_ready_done

                loop     .wait_ready_loop

                ; timeout for BSY = 0 or RDY = 1

        .wait_ready_err_loop:

                in      al, dx

                test    al, IDE_DEVICE_BUSY_FLAG ; BSY = 0
                jnz     .wait_ready_err_loop

                test    al, IDE_DEVICE_ERROR_FLAG
                jz      .wait_ready_done

                xor     bl, bl           ; ERR = 1 or DF = 1

        .wait_ready_done:

                mov     al, bl

                pop     rbx
                pop     rcx
                pop     rdx
                ret

; dx <- base port
; al -> 1 - drq awaited, 0 - error or timeout

wait_ide_device_drq:

                push    rdx
                push    rcx
                push    rbx

                mov     bl, 1
                add     dx, IDE_STATUS_COMMAND_REG

                mov     rcx, 0xFFFF

        .wait_device_drq_loop:

                in      al, dx

                test    al, IDE_DEVICE_BUSY_FLAG    ; wait BSY = 0
                jz      .wait_device_drq_not_busy

                loop    .wait_device_drq_loop

                jmp     .wait_device_drq_err_loop   ; timeout for BSY = 0

        .wait_device_drq_not_busy:

                test    al, IDE_DEVICE_DRQ_FLAG     ; wait DRQ = 1
                jnz     .wait_device_drq_done
        
                loop    .wait_device_drq_loop

                ; timeout for BSY = 0 or DRQ = 1, check error

        .wait_device_drq_err_loop:

                in      al, dx

                test    al, IDE_DEVICE_BUSY_FLAG
                jnz     .wait_device_drq_err_loop 

                test    al, IDE_DEVICE_ERROR_FLAG
                jz      .wait_device_drq_done

                xor     bl, bl

        .wait_device_drq_done:

                mov     al, bl

                pop     rbx
                pop     rcx
                pop     rdx
                ret