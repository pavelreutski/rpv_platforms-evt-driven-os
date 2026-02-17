        .text

        /* ICahe */

        .globl _mb_icache_enable
        .ent _mb_icache_enable
        .align 2
_mb_icache_enable:

        mfs     r8, rmsr                    
        ori     r8, r8, 0x20
        mts     rmsr, r8                    /* enable icache */

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
        mts     rmsr, r8                    /* enable dcache */

        rtsd    r15, 8
        nop

        .end    _mb_dcache_enable

        /* enable exceptions */

        .globl _mb_exceptions_enable
        .ent _mb_exceptions_enable
        .align 2
_mb_exceptions_enable:

        mfs     r8, rmsr
        ori     r8, r8, 0x100
        mts     rmsr, r8                    /* set exceptions enable flag */

        rtsd    r15, 8
        nop

        .end    _mb_exceptions_enable

        /* enable interrupts */

        .globl  _mb_interrupts_enable
        .ent _mb_interrupts_enable
        .align 2
_mb_interrupts_enable:

        mfs     r8, rmsr
        ori     r8, r8, 0x02
        mts     rmsr, r8                    /* set global interrupts enable flag */

        rtsd    r15, 8
        nop

        .end    _mb_interrupts_enable

        /* L2 cache invalidate range */

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

        /* L1 cache invalidate range */
        
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

        /* L2 cache flush range */

        .globl _mb_l2_cache_flush_range
        .ent _mb_l2_cache_flush_range
        .align 2
_mb_l2_cache_flush_range:

        beqi    r6, .mb_l2_flush_done

        addik   r6, r6, -1
        add     r6, r5, r6

        andi    r6, r6, -(4 * 16)
        andi    r5, r5, -(4 * 16)

        rsubk   r6, r5, r6

.mb_l2_flush_start:

        wdc.ext.flush   r5, r6

        bneid   r6, .mb_l2_flush_start
        addik   r6, r6, -(4 * 16)

.mb_l2_flush_done:

        rtsd    r15, 8
        nop

        .end _mb_l2_cache_flush_range

        /* L1 cache flush range */

        .globl _mb_l1_cache_flush_range
        .ent _mb_l1_cache_flush_range
        .align 2
_mb_l1_cache_flush_range:

        beqi    r6, .mb_l1_flush_done

.mb_l1_flush_done:

        rtsd    r15, 8
        nop

        .end _mb_l1_cache_flush_range