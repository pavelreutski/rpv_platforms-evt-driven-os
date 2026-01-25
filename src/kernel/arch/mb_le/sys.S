        .text

/* ICahe */

        .globl _mb_icache_enable
        .ent _mb_icache_enable
        .align 2
_mb_icache_enable:

        mfs     r8, rmsr                    
        ori     r8, r8, 0x20
        mts     rmsr, r8                    /* Enable ICache */

        rtsd    r15, 8
        nop

        .end    _mb_icache_enable

/* DCache */

        .globl _mb_dcache_enable
        .ent _mb_dcache_enable
        .align 2
_mb_dcache_enable:

        mfs     r8, rmsr
        ori     r8, r8, 0x80
        mts     rmsr, r8                    /* Enable DCache */

        rtsd    r15, 8
        nop

        .end    _mb_dcache_enable

/* exceptions and interrupts */

        .globl _mb_exceptions_enable
        .ent _mb_exceptions_enable
        .align 2
_mb_exceptions_enable:

        mfs     r8, rmsr
        ori     r8, r8, 0x100
        mts     rmsr, r8                    /* Enable exceptions */

        rtsd    r15, 8
        nop

        .end    _mb_exceptions_enable

        .globl  _mb_interrupts_enable
        .ent _mb_interrupts_enable
        .align 2
_mb_interrupts_enable:

        mfs     r8, rmsr
        ori     r8, r8, 0x02
        mts     rmsr, r8                    /* Enable global interrupts */

        rtsd    r15, 8
        nop

        .end    _mb_interrupts_enable
        
        