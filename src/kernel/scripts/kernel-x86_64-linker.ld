ENTRY(start)

PHDRS {
    text PT_LOAD FLAGS(5);   /* R + X */
    data PT_LOAD FLAGS(6);   /* R + W */
}

SECTIONS {

    /* Kernel load address */
    . = 1M;

    /* ===== Boot header (must be first, must be in an RX segment) ===== */
    .boot : 
    {
        KEEP(*(.multiboot2))
    } : text

    /* ===== Text / code / read-only data ===== */
    .text : ALIGN(16)
    {
        *(.text*)
        *(.rodata*)
    } : text

    /* ===== Initialized data ===== */
    .data : ALIGN(16)
    {
        *(.data*)
    } :data

    /* ===== Uninitialized data ===== */
    .bss : ALIGN(16)
    {
        *(COMMON)
        *(.bss*)
    } :data
}