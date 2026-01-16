cmake_minimum_required(VERSION 3.25.0)

find_program(AVR_UPLOADTOOL avrdude REQUIRED)

function(program_mcu_on_usbasp EXECUTABLE AVR_MCU_MODEL AVR_UPLOADTOOL_PART)

	set(hex_file              ${EXECUTABLE}.hex)
	set(upload_options        -c usbasp -p ${AVR_UPLOADTOOL_PART})

	add_custom_target(
		  usbasp_program_mcu_exec
		  ${AVR_UPLOADTOOL} ${upload_options}
			 -U flash:w:${hex_file}:i
		  DEPENDS ${EXECUTABLE}
		  COMMENT "Uploading ${hex_file} to ${AVR_MCU_MODEL} using USBasp")

endfunction(program_mcu_on_usbasp)

function(program_mcu_on_arduino ELF_TARGET AVR_MCU_MODEL AVR_UPLOADTOOL_PART)

	if(NOT TARGET ${ELF_TARGET})
        message(FATAL_ERROR "Kernel target '${ELF_TARGET}' does not exist.")
    endif()

	set(hex_file			$<TARGET_FILE_DIR:${ELF_TARGET}>/${ELF_TARGET}.hex)
	set(upload_options      -c arduino -P /dev/ttyACM0 -b115200 -p ${AVR_UPLOADTOOL_PART})

	add_custom_target(
		program_arduino_mcu
		${AVR_UPLOADTOOL} ${upload_options}
			-U flash:w:${hex_file}:i
		DEPENDS ${EXECUTABLE}
		COMMENT "Uploading ${hex_file} to ${AVR_MCU_MODEL} using Arduino")

endfunction(program_mcu_on_arduino)

function(program_mcu_fuse AVR_MCU_MODEL AVR_UPLOADTOOL_PART AVR_PROG_L_FUSE AVR_PROG_H_FUSE)

	set(upload_options        -c usbasp -p ${AVR_UPLOADTOOL_PART})

	add_custom_target(
		  program_mcu_fuse
		  ${AVR_UPLOADTOOL} ${upload_options}
			 -U lfuse:w:${AVR_PROG_L_FUSE}:m
			 -U hfuse:w:${AVR_PROG_H_FUSE}:m
		  COMMENT "Program fuse (low: ${AVR_PROG_L_FUSE}, high: ${AVR_PROG_H_FUSE}) to ${AVR_MCU_MODEL} using USBasp")

endfunction(program_mcu_fuse)