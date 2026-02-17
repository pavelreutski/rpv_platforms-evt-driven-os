        .globl _crtinit
        .ent _crtinit
        .align 2
_crtinit:
        addi	r1, r1, -20                 /* Save Link register	 */
        swi	r15, r1, 0

        addi	r6, r0, __sbss_start        /* clear SBSS */
        addi	r7, r0, __sbss_end	
        rsub	r18, r6, r7		
        blei	r18, .Lendsbss

.Lloopsbss:	
        swi	r0, r6, 0
        addi	r6, r6, 4
        rsub	r18, r6, r7
        bgti	r18, .Lloopsbss
.Lendsbss:

        addi	r6, r0, __bss_start         /* clear BSS */
        addi	r7, r0, __bss_end		
        rsub	r18, r6, r7		
        blei	r18, .Lendbss
.Lloopbss:	
        swi	r0, r6, 0
        addi	r6, r6, 4
        rsub	r18, r6, r7
        bgti	r18, .Lloopbss
.Lendbss:

        rtsd	r15, 8
        addi	r1, r1, 20

        .end _crtinit