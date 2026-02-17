global _page_table_l2
global _page_table_l3
global _page_table_l4

global _page_table_mmio

[section .bss]

align 4096 ; 4K page boundary alignment

_page_table_l4:
    resb    4096

_page_table_l3:
    resb    4096

_page_table_l2:
    resb    4096

_page_table_mmio:  ; page table for MMIO - 4Mb space for huge pages
    resb    16
