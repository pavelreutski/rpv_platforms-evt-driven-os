global page_table_l2, page_table_l3, page_table_l4

[section .bss]

align 4096 ; 4K page boundary alignment

page_table_l4:
    resb    4096

page_table_l3:
    resb    4096

page_table_l2:
    resb    4096
