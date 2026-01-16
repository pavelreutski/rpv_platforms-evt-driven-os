# Set microcontroller information

set(MCU_H_FUSE 0xC9)
set(MCU_L_FUSE 0xFF)
set(MCU_E_FUSE 0x00)

set(MCU_FREQ   16000000ul)
set(MCU_MODEL  atmega328p)

set(MCU_UPLOADTOOL_PART m328p)

add_link_options(-mmcu=${MCU_MODEL})
add_compile_options(-mmcu=${MCU_MODEL} -O2 -g -gdwarf-4 
                    -fdata-sections -ffunction-sections 
                    -Wl,--gc-sections
 		            -fpack-struct -fshort-enums
		            -Wall -Werror -Wextra
		            -Wl,--start-group
		            -Wl,--end-group
		            -pedantic -pedantic-errors
		            -funsigned-char -funsigned-bitfields)

function(avr_elf_dump ELF_TARGET)

    if(NOT TARGET ${ELF_TARGET})
        message(FATAL_ERROR "Kernel target '${ELF_TARGET}' does not exist.")
    endif()

    # Path for the raw binary
    set(ELF_FILE $<TARGET_FILE:${ELF_TARGET}>)
	set(ELF_IHEX_FILE $<TARGET_FILE_DIR:${ELF_TARGET}>/${ELF_TARGET}.hex)

	add_custom_target(${ELF_TARGET}-elf-avr-utils ALL
		COMMAND ${CMAKE_SIZE} ${ELF_FILE}
		COMMAND ${CMAKE_OBJCOPY} -O ihex ${ELF_FILE} ${ELF_IHEX_FILE}		
        DEPENDS ${ELF_TARGET}
        COMMENT "running avr elf utils for ${ELF_TARGET}"
    )

endfunction()