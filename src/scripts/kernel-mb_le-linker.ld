/* Specify the default entry point to the program */

ENTRY(_start)

_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x8000;
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x800;

/*
 * Memory map:
 *   BRAM: 32 KiB at 0x00000000
 *   DDR2: 96 MiB at 0x82000000
 *
 * The first 0x50 bytes of BRAM are reserved for MicroBlaze vectors.
 * The reset vector at 0x00000000 belongs to the permanent BRAM
 * boot stub/bootloader, not to the DDR2 application.
 */

MEMORY
{
   vector_memory   : ORIGIN = 0x00000000, LENGTH = 0x00000050 /* 80 b */
   local_memory    : ORIGIN = 0x00000050, LENGTH = 0x00007FB0 /* 32 Kb */
   external_memory : ORIGIN = 0x82000000, LENGTH = 0x06000000 /* 96 Mb */
}

SECTIONS
{
   /*
    * Application exception vectors in BRAM.
    * Do not define .vectors.reset here: address 0x00000000
    * is reserved for the permanent boot stub/bootloader.
    */

   .vectors.sw_exception 0x00000008 : { KEEP (*(.vectors.sw_exception)) } > vector_memory
   .vectors.interrupt 0x00000010 : { KEEP (*(.vectors.interrupt)) } > vector_memory
   .vectors.hw_exception 0x00000020 : { KEEP (*(.vectors.hw_exception)) } > vector_memory

   .init : { KEEP (*(.init)) } > external_memory
   .fini : { KEEP (*(.fini)) } > external_memory

   /*
    * Application code and read-only data in external memory.
    */

   .text : {
      *(.text)
      *(.text.*)
      *(.gnu.linkonce.t.*)
   } > external_memory

   .rodata : {
      __rodata_start = .;
      *(.rodata)
      *(.rodata.*)
      *(.gnu.linkonce.r.*)
      __rodata_end = .;
   } > external_memory  

   .data : {
      . = ALIGN(4);
      __data_start = .;
      *(.data)
      *(.data.*)
      *(.gnu.linkonce.d.*)

      /* Commands section */

      . = ALIGN(4); /* ensure 4 bytes alignment */

      __cmd_table = .;
      KEEP(*(.cmd_table))
      __end_cmd_table = .;   

      /* Services section */

      __svcinit_table = .;
      KEEP(*(.service_init))
      __end_svcinit_table = .;

      __svc_table = .;
      KEEP(*(.service_table))
      __end_svc_table = .;

      __data_end = .;
   } > external_memory
   
   .sdata2 : {
      . = ALIGN(8);
      __sdata2_start = .;
      *(.sdata2)
      *(.sdata2.*)
      *(.gnu.linkonce.s2.*)
      . = ALIGN(8);
      __sdata2_end = .;
   } > external_memory
   
   .sbss2 (NOLOAD) :
    {
        . = ALIGN(8);
        __sbss2_start = .;

        *(.sbss2)
        *(.sbss2.*)
        *(.gnu.linkonce.sb2.*)

        . = ALIGN(8);
        __sbss2_end = .;
    } > external_memory   

   .sdata : {
      . = ALIGN(8);
      __sdata_start = .;
      *(.sdata)
      *(.sdata.*)
      *(.gnu.linkonce.s.*)
      __sdata_end = .;
   } > external_memory

   /*
    * Uninitialized data. The CRT must clear this range.
    */
   .sbss (NOLOAD) : {
      . = ALIGN(4);
      __sbss_start = .;
      *(.sbss)
      *(.sbss.*)
      *(.gnu.linkonce.sb.*)
      . = ALIGN(8);
      __sbss_end = .;
   } > external_memory   

   .bss (NOLOAD) : {
      . = ALIGN(4);
      __bss_start = .;
      *(.bss)
      *(.bss.*)
      *(.gnu.linkonce.b.*)
      *(COMMON)
      . = ALIGN(4);
      __bss_end = .;
   } > external_memory

   .eh_frame : {
      . = ALIGN(4); 
      __eh_frame_start = .;
      KEEP(*(.eh_frame))
      KEEP(*(.eh_frame.*))
      __eh_frame_end = .;
    } > external_memory

   /*
    * MicroBlaze small-data base addresses.
    */
   _SDA_BASE_ = __sdata_start + ((__sbss_end - __sdata_start) / 2 );
   _SDA2_BASE_ = __sdata2_start + ((__sbss2_end - __sdata2_start) / 2 );

   /*
    * Heap and stack in external memory.
    */
   .heap (NOLOAD) : {
      . = ALIGN(8);
      _heap = .;
      _heap_start = .;
      . += _HEAP_SIZE;
      _heap_end = .;
   } > external_memory

   .stack (NOLOAD) : {
      _stack_end = .;
      . += _STACK_SIZE;
      . = ALIGN(8);
      _stack = .;
      __stack = _stack;
   } > external_memory

   _end = .;

   ASSERT(_end <= ORIGIN(external_memory) + LENGTH(external_memory),
           "DDR2 application exceeds external_memory")
}