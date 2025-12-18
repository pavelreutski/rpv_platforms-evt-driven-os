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
    set(IDE_DISK_FILE $<TARGET_FILE_DIR:${ISO_TARGET}>/ide_disk.img)

    add_custom_target(qemu-x86_64-idedisk
        COMMAND qemu-img create -f raw ${IDE_DISK_FILE} 512M
        DEPENDS ${ISO_TARGET}
        COMMENT "Create IDE disk image. Disk size: 512 Mb")

    add_custom_target(qemu-x86_64-gdb
        COMMAND qemu-system-x86_64 -m 512M -cdrom ${ISO_FILE} -drive file=${IDE_DISK_FILE},if=ide,index=0,media=disk,format=raw -s -S
        DEPENDS qemu-x86_64-idedisk
        COMMENT "Debug kernel ISO via QEMU"
    )

    add_custom_target(qemu-x86_64-run
        COMMAND qemu-system-x86_64 -m 512M -cdrom ${ISO_FILE} -drive file=${IDE_DISK_FILE},if=ide,index=0,media=disk,format=raw
        DEPENDS qemu-x86_64-idedisk
        COMMENT "Run kernel ISO in QEMU"
    )

endfunction()