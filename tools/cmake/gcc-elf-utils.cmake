# Function:
#   gdb_qemu_iso(<iso-target>)
#
# Only handles copying the kernel and generating GRUB ISO.
# Does NOT create run/debug targets.

function(elf_dump ELF_TARGET)

    if(NOT TARGET ${ELF_TARGET})
        message(FATAL_ERROR "Kernel target '${ELF_TARGET}' does not exist.")
    endif()

    # Path for the raw binary
    set(ELF_FILE $<TARGET_FILE:${ELF_TARGET}>)
    set(ELF_SECTIONS_FILE $<TARGET_FILE_DIR:${ELF_TARGET}>/${ELF_TARGET}.sec)
    set(ELF_RAW_BIN_FILE $<TARGET_FILE_DIR:${ELF_TARGET}>/${ELF_TARGET}.bin)

    # Custom target: convert ELF -> binary run with build

    add_custom_target(${ELF_TARGET}-elf-utils ALL
        COMMAND ${CMAKE_OBJCOPY} -O binary ${ELF_FILE} ${ELF_RAW_BIN_FILE}
        COMMAND ${CMAKE_OBJDUMP} -h ${ELF_FILE} > ${ELF_SECTIONS_FILE}        
        DEPENDS ${ELF_TARGET}
        COMMENT "running elf utils for ${ELF_TARGET}"
    )    

endfunction()