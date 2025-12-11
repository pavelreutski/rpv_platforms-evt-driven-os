global _stack_top

[section .bss]

align 16

stack_bottom:
    resb    16384 ; 16K for stack allocated
_stack_top: