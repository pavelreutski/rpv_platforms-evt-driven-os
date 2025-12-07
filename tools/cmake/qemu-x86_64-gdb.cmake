# Function:
#   gdb_qemu_iso(<iso-target>)
#
# Only handles copying the kernel and generating GRUB ISO.
# Does NOT create run/debug targets.

function(gdb_qemu_iso ISO_TARGET)

    if(NOT TARGET ${ISO_TARGET})
        message(FATAL_ERROR "Kernel target '${ISO_TARGET}' does not exist.")
    endif()

    set(ISO_FILE $<TARGET_FILE_DIR:${ISO_TARGET}>/${ISO_TARGET}.iso)

    add_custom_target(qemu-x86_64-gdb
        COMMAND qemu-system-x86_64 -cdrom ${ISO_FILE} -s -S
        DEPENDS ${ISO_TARGET}
        COMMENT "Debug kernel ISO via QEMU"
    )

    add_custom_target(qemu-x86_64-run
        COMMAND qemu-system-x86_64 -cdrom ${ISO_FILE}
        DEPENDS ${ISO_TARGET}
        COMMENT "Run kernel ISO in QEMU"
    )

endfunction()