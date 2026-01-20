/*      
        MicroBlaze Vector Map for standalone executables

         Address                Vector type                 Label
         -------                -----------                 ------    

    	# 0x00 #		        (-- IMM --)
	    # 0x04 #                Reset                       _start1
    
	    # 0x08 #		        (-- IMM --)
	    # 0x0c #		        Software Exception          _exception_handler
    
	    # 0x10 #		        (-- IMM --)
	    # 0x14 #		        Hardware Interrupt          _interrupt_handler
    
        # 0x18 #                (-- IMM --)
        # 0x1C #                Breakpoint Exception        (-- Don't Care --)
    
        # 0x20 #                (-- IMM --) 
        # 0x24 #                Hardware Exception          _hw_exception_handler
*/

	.globl _start
        .section .vectors.reset, "ax"
	.align 2
        .ent _start
        .type _start, @function
_start:
        brai    _start1
        .end _start
    
        .section .vectors.sw_exception, "ax"
        .align 2
_vector_sw_exception:       
        bri     0     /* _exception_handler */

        .section .vectors.interrupt, "ax"
        .align 2
_vector_interrupt:      
        bri     0    /* _interrupt_handler */

        .section .vectors.hw_exception, "ax"
        .align 2
_vector_hw_exception:       
        bri     0     /* _hw_exception_handler */

        .section .text
        .globl _start1
        .align 2
        .ent _start1
        .type _start1, @function   
_start1:
	    la	r13, r0, _SDA_BASE_         /* Set the Small Data Anchors and the stack pointer */
	    la	r2, r0, _SDA2_BASE_
	    la	r1, r0, _stack-16           /* 16 bytes (4 words are needed by crtinit for args and link reg */

	    brlid	r15, _crtinit               /* Initialize BSS and run program */
	    nop

        brlid   r15, exit                   /* Call exit with the return value of main */
        addik   r5, r3, 0                   

        /* Control does not reach here */
        .end _start1
/* 
        _exit 
        Our simple _exit      
*/    
        .globl _exit
        .align 2
        .ent _exit
        .type _exit, @function    
_exit:
        bri     0
	.end _exit        
