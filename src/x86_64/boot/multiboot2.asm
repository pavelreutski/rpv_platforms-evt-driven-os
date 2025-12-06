; Multiboot2 header must be in first 32 Kb of kernel and aligned at 8 bytes

[section .multiboot2]

align 8

MB2_MAGIC           equ 0xE85250D6 ; Multiboot2
MB2_ARCH            equ 0          ; protected mode i386
MB2_HEADER_LEN      equ (header_end - multiboot2_header)
MB2_CHECKSUM        equ -(MB2_MAGIC + MB2_ARCH + MB2_HEADER_LEN)

multiboot2_header:

    dd      MB2_MAGIC      ; Magic number
    dd      MB2_ARCH       ; architecture
    dd      MB2_HEADER_LEN ; header length
    dd      MB2_CHECKSUM   ; checksum

    ; ===== End tag (required) =====

    dw      0              ; type (end tag)
    dw      0              ; flags
    dd      8              ; size = 8

header_end: