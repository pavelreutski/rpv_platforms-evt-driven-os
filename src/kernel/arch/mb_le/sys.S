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

        .globl _mb_l2_cache_invalidate_range
        .ent _mb_l2_cache_invalidate_range
        .align 2
_mb_l2_cache_invalidate_range:

        beqi    r6, .mb_l2_inv_done

        add     r6, r5, r6
        addik   r6, r6, -1

        andi    r6, r6, -(4 * 16)
        andi    r5, r5, -(4 * 16)

        rsubk   r6, r5, r6

.mb_l2_inv_start:
        
        wdc.ext.clear   r5, r6     

        bneid   r6, .mb_l2_inv_start
        addik   r6, r6, -(4 * 16)

.mb_l2_inv_done:

        rtsd    r15, 8
        nop

        .end    _mb_l2_cache_invalidate_range
        
        .globl _mb_l1_cache_invalidate_range
        .ent _mb_l1_cache_invalidate_range
        .align 2
_mb_l1_cache_invalidate_range:

        beqi    r6, .mb_l1_inv_done

        add     r6, r5, r6
        addik   r6, r6, -1

        andi    r6, r6, -(4 * 4)
        andi    r5, r5, -(4 * 4)

.mb_l1_inv_start:

        cmpu    r18, r5, r6
        blti    r18, .mb_l1_inv_done

        wdc     r5, r0

        brid    .mb_l1_inv_start
        addik   r5, r5, (4 * 4)

.mb_l1_inv_done:

        rtsd    r15, 8
        nop

        .end _mb_l1_cache_invalidate_range