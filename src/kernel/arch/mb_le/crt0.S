/*      
        MicroBlaze Vector Map for standalone executables

         Address                Vector type                 Label
         -------                -----------                 ------    

    	# 0x00 #		(-- IMM --)
	# 0x04 #                Reset                       _start1
    
	# 0x08 #		(-- IMM --)
	# 0x0c #		Software Exception          _exception_handler
    
	# 0x10 #		(-- IMM --)
	# 0x14 #		Hardware Interrupt          _interrupt_handler
    
        # 0x18 #                (-- IMM --)
        # 0x1C #                Breakpoint Exception        (-- Don't Care --)
    
        # 0x20 #                (-- IMM --) 
        # 0x24 #                Hardware Exception          _hw_exception_handler
*/
	.globl _start
        .section .vectors.reset, "ax"
        .ent _start
        .align 2
_start:
        brai    _start1 /* jump to start at reset */
        .end _start
    
        .section .vectors.sw_exception, "ax"
        .align 2
_vector_sw_exception:       
        brai    _exception_handler       /* software exceptions */

        .section .vectors.interrupt, "ax"
        .align 2
_vector_interrupt:      
        brai    _interrupt_handler       /* none fast interrupts */

        .section .vectors.hw_exception, "ax"
        .align 2
_vector_hw_exception:       
        brai    _hw_exception_handler    /* hardware exceptions */

        .section .text

        .globl _start1
        .ent _start1
        .align 2 
_start1:
	la	r13, r0, _SDA_BASE_         /* Set the Small Data Anchors and the stack pointer */
	la	r2, r0, _SDA2_BASE_
	la	r1, r0, _stack              

	brlid	r15, _crtinit               /* Initialize BSS */
	nop

        brlid   r15, main                   /* Call main */
        nop

.exit:  bri     0                           /* Loop forever since main shouldn`t return */              

        .end _start1

/* Software exception */

        .globl _exception_handler
        .ent _exception_handler
        .align 2

_exception_handler:

        addik   r1, r1, -12

        swi     r5, r1, 0
        swi     r6, r1, 4

        mfs     r6, resr
        andi    r6, r6, 0x1000         /* DS mask of esr */
        BEQI    r6, .ex_swhandler_no_ds
        mfs     r17, rbtr

.ex_swhandler_no_ds:

        swi     r17, r1, 8

        /* call software exception handler */

        mfs     r5, rear
        mfs     r6, resr

        brlid   r15, _xsoft_except
        nop

        lwi     r17, r1, 8
        lwi     r6, r1, 4
        lwi     r5, r1, 0
        
        addik   r1, r1, 12

        rted    r17, 0
        nop

        .end _exception_handler

/* Interrupt handler */

        .globl _interrupt_handler
        .ent _interrupt_handler
        .align 2
_interrupt_handler: bri 0
        .end _interrupt_handler

/* Hardware exception */

        .globl _hw_exception_handler
        .ent _hw_exception_handler
        .align 2
_hw_exception_handler:

        addik   r1, r1, -12

        swi     r5, r1, 0
        swi     r6, r1, 4

        mfs     r6, resr
        andi    r6, r6, 0x1000         /* DS mask of esr */
        BEQI    r6, .ex_hwhandler_no_ds
        mfs     r17, rbtr

.ex_hwhandler_no_ds:

        swi     r17, r1, 8

        /* call hardware exception handler */

        mfs     r5, rear
        mfs     r6, resr

        brlid   r15, _xhard_except
        nop

        lwi     r17, r1, 8
        lwi     r6, r1, 4
        lwi     r5, r1, 0

        addik   r1, r1, 12

        rted    r17, 0
        nop

        .end _hw_exception_handler
