ENTRY(_start)
OUTPUT_ARCH(i386:x86-64)

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

    /* ==== commands table ======= */

    .cmd_table :
    {
        __cmd_table = .;
        KEEP(*(.cmd_table))
        __end_cmd_table = .;
    } :data
}