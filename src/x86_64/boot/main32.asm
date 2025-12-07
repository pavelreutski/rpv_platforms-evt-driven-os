global _start

extern long_mode_start

extern stack_top, gdt64.pointer, gdt64.code_segment, page_table_l2, page_table_l3, page_table_l4

NO_CPUID_CODE       equ     "C"
NO_MULTIBOOT_CODE   equ     "M"
NO_LONG_MODE_CODE   equ     "L"

[section .text]

[bits 32]

_start:

    mov     esp, stack_top

    call    check_multiboot
    call    check_cpuid
    call    check_long_mode

    call    setup_page_tables    
    call    enable_paging

    lgdt    [gdt64.pointer]
    jmp     gdt64.code_segment:long_mode_start

check_multiboot:
    cmp     eax, 0x36d76289
    jnz     .no_multiboot
    ret

.no_multiboot:
    mov     al, NO_MULTIBOOT_CODE
    jmp     error

check_cpuid:
    pushfd
    pop     eax            ; eax <- cpu flags
    mov     ecx, eax

    xor     eax, (1 << 21) ; flip id bit
    push    eax
    popfd                  ; set flags

    pushfd
    pop     eax            ; eax <- cpu flags back

    push    ecx
    popfd                  ; restore cpu flags

    cmp     eax, ecx
    jz      .no_cpuid
    ret

.no_cpuid:
    mov     al, NO_CPUID_CODE
    jmp     error

check_long_mode:
    mov     eax, 0x80000000 ; <- query extended cpuid
    cpuid

    cmp     eax, 0x80000001 ; <- check if its available
    jb      .no_long_mode

    mov     eax, 0x80000001 ; <- get extended cpuid
    cpuid

    test    edx, (1 << 29) ; <- test for LM bit
    jz      .no_long_mode
    ret

.no_long_mode:
    mov     al, NO_LONG_MODE_CODE
    jmp     error

setup_page_tables:
    mov     eax, page_table_l3
    or      eax, 0b11          ; enable present and writable flags
    mov     [page_table_l4], eax

    mov     eax, page_table_l2
    or      eax, 0b11          ; enable present and writable flags
    mov     [page_table_l3], eax

    mov     ecx, 0
.loop:

    mov     eax, 0x200000       ; 2 Mb
    mul     ecx

    or      eax, 0b10000011                 ; enable present & writable & huge page flags                 
    mov     [page_table_l2 + ecx * 8], eax

    inc     ecx

    cmp     ecx, 512            ; check if a whole table is mapped
    jnz     .loop
    ret

enable_paging:
    mov     eax, page_table_l4
    mov     cr3, eax           ; pass page table location to cpu

    ; enable PAE
    mov     eax, cr4
    or      eax, (1 << 5)      ; enable PAE flag
    mov     cr4, eax

    ; enable long mode
    mov     ecx, 0xC0000080                 
    rdmsr
    or      eax, (1 << 8)      ; enable long mode flag
    wrmsr    

    ; enable paging
    mov     eax, cr0
    or      eax, (1 << 31)     ; enable paging flag    
    mov     cr0, eax

    ret

error:

    mov     edi, 0xb8000

    mov     dword [edi],     0x4f524f45 ; ER
    mov     dword [edi + 4], 0x4f3a4f52 ; R:
    mov     dword [edi + 8], 0x4f204f20 ; <sp><sp>

    mov     byte [edi + 10], al ; error code

    hlt