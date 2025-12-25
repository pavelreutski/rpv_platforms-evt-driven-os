# Function:
#   kernel_iso(<kernel-target>)
#
# Only handles copying the kernel generating GRUB ISO and IDE disk images

function(kernel_iso KERNEL_TARGET)

    if(NOT TARGET ${KERNEL_TARGET})
        message(FATAL_ERROR "Kernel target '${KERNEL_TARGET}' does not exist.")
    endif()

    set(ISO_DIR      "${CMAKE_SOURCE_DIR}/targets/x86_64/iso")
    set(ISO_BOOT_DIR "${ISO_DIR}/boot")
    set(ISO_OUTPUT   "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${KERNEL_TARGET}.iso")

    set(KERNEL_ELF $<TARGET_FILE:${KERNEL_TARGET}>)

    #
    # Step 1: Copy kernel elf into ISO structure
    #
    add_custom_command(
        OUTPUT ${ISO_BOOT_DIR}/kernel.bin
        DEPENDS ${KERNEL_TARGET}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${ISO_BOOT_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy ${KERNEL_ELF} ${ISO_BOOT_DIR}/kernel.bin
        COMMENT "Copying kernel elf to ${ISO_BOOT_DIR}"
    )

    #
    # Step 2: Create ISO
    #
    add_custom_command(
        OUTPUT ${ISO_OUTPUT}
        DEPENDS ${ISO_BOOT_DIR}/kernel.bin
        COMMAND grub-mkrescue -o ${ISO_OUTPUT} ${ISO_DIR}
        COMMENT "Generating GRUB ISO image: ${ISO_OUTPUT}"
    )

    #
    # ISO target
    #
    add_custom_target(iso
        DEPENDS ${ISO_OUTPUT}
        COMMENT "ISO image created at ${ISO_OUTPUT}"
    )

endfunction()
