global _start

; x86-64 entry
extern _long_mode_start

; stack
extern _stack_top

; page tables

extern _page_table_l3
extern _page_table_l4
extern _page_table_l2

extern _page_table_mmio

; GDT64
extern gdt64.pointer, gdt64.code_segment

NO_CPUID_CODE       equ     "C"
NO_LAPIC_CODE       equ     "A"
NO_MULTIBOOT_CODE   equ     "M"
NO_LONG_MODE_CODE   equ     "L"

[section .text]

[bits 32]

_start:

    mov     esp, _stack_top

    call    check_multiboot
    call    check_cpuid

    call    check_lapic
    call    check_long_mode

    call    setup_page_tables    
    call    enable_paging

    lgdt    [gdt64.pointer]
    jmp     gdt64.code_segment:_long_mode_start

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

check_lapic:
    mov     eax, 1
    cpuid

    test    edx, (1 << 9) ; <- test for LAPIC bit
    jz      .no_lapic
    ret

.no_lapic:
    mov     al, NO_LAPIC_CODE
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
    mov     eax, _page_table_l3
    or      eax, 0b11          ; enable present and read/write flags
    mov     [_page_table_l4], eax

    mov     eax, _page_table_l2
    or      eax, 0b11          ; enable present and read/write flags
    mov     [_page_table_l3], eax

    ; general memory (1 Gb)
    
    ; virtual: 0x00000000–0x3FFFFFFF
    ; physical: 0x00000000–0x3FFFFFFF

    xor     ecx, ecx
    mov     edi, _page_table_l2

.map_gmem_loop:

    mov     eax, 0x200000       ; 2 Mb
    mul     ecx

    or      eax, 0b10000011     ; enable present & read/write & huge page flags                 
    mov     [edi + ecx * 8], eax

    inc     ecx

    cmp     ecx, 512            ; check if a whole table is mapped
    jnz     .map_gmem_loop

    ; MMIO memory (4 Mb)

    ; virtual : 0x40000000 - 0x403FFFFF
    ; physical: 0xFEC00000 - 0xFEFFFFFF

    mov     edi, _page_table_l3
    mov     eax, _page_table_mmio
    or      eax, 0b11           ; enable present & read/write page flags

    mov     [edi + 8], eax      ; second entry of a page directory

    xor     ecx, ecx
    mov     edi, _page_table_mmio

.map_mmio_loop:

    mov     eax, 0x200000       ; 2 Mb
    mul     ecx
    add     eax, 0xFEC00000

    or      eax, 0b10010011     ; enable present & read/write & none cachable & huge page flags
    mov     [edi + ecx * 8], eax

    inc     ecx

    cmp     ecx, 2
    jnz     .map_mmio_loop

    ret

enable_paging:
    mov     eax, _page_table_l4
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

.hlt_loop:
    hlt
    jmp     .hlt_loop