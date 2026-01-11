global gdt64.pointer, gdt64.code_segment

[section .rodata]

gdt64:

    dq      0 ; zero entry

.code_segment: equ ($ - gdt64)
    dq      (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53) ; code segment

.pointer:
    dw      ($ - gdt64 - 1) ; size of GDT
    dq      gdt64           ; address of GDT