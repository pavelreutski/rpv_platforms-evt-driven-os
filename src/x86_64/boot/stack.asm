global stack_top

[section .bss]

stack_bottom:
    resb    16384 ; 16K for stack allocated
stack_top: