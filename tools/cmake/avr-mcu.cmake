# Set microcontroller information

set(MCU_H_FUSE 0xC9)
set(MCU_L_FUSE 0xFF)
set(MCU_E_FUSE 0x00)

set(MCU_FREQ   16000000ul)
set(MCU_MODEL  atmega328p)

set(MCU_UPLOADTOOL_PART m328p)

add_link_options(-mmcu=${MCU_MODEL} -Wl,--print-memory-usage)
add_compile_options(-mmcu=${MCU_MODEL} -c -O2 -g -std=c23
                    -fdata-sections -ffunction-sections 
                    -Wl,--gc-sections
 		            -fpack-struct -fshort-enums
		            -Wall -Werror
		            -Wl,--start-group
		            -Wl,--end-group
		            -pedantic -pedantic-errors
		            -funsigned-char -funsigned-bitfields)