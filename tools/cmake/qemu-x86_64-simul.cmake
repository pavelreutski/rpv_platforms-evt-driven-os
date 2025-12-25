# Function:
#   qemu-simul (<kernel-target>)
#
# Only handles copying the kernel and generating GRUB ISO.
# Does NOT create run/debug targets.

function(qemu_simul KERNEL_TARGET)

    if(NOT TARGET ${KERNEL_TARGET})
        message(FATAL_ERROR "Kernel target '${KERNEL_TARGET}' does not exist.")
    endif()
    
    set(ISO_FILE $<TARGET_FILE_DIR:${KERNEL_TARGET}>/${KERNEL_TARGET}.iso)
    set(IDE_DISK_FILE $<TARGET_FILE_DIR:${KERNEL_TARGET}>/ide_disk.img)

    add_custom_target(qemu-x86_64-idedisk
        COMMENT "Create 20 MB IDE disk image with FAT16 partition and test file"
        DEPENDS ${KERNEL_TARGET}

        # 1. Create empty 20 MB raw disk
        COMMAND echo "[1/5] Creating 20 MB raw disk..."
        COMMAND dd if=/dev/zero of=${IDE_DISK_FILE} bs=1M count=20 status=progress

        # 2. Create MBR partition (FAT16, bootable)
        COMMAND echo "[2/5] Creating MBR and FAT16 partition..."
        COMMAND parted ${IDE_DISK_FILE} --script mklabel msdos mkpart primary fat16 1MiB 100% set 1 boot on

        # 3. Create partition image and format as FAT16
        COMMAND echo "[3/5] Creating and formatting partition image..."
        COMMAND dd if=/dev/zero of=${CMAKE_BINARY_DIR}/part.img bs=1M count=19 status=progress
        COMMAND mkfs.vfat -F16 -n "DISK#0" ${CMAKE_BINARY_DIR}/part.img

        # 4. Inject test file into partition image
        COMMAND echo "[4/5] Injecting test file..."
        COMMAND echo "Hello from 20MB FAT16 disk" > ${CMAKE_BINARY_DIR}/test.txt
        COMMAND mcopy -i ${CMAKE_BINARY_DIR}/part.img ${CMAKE_BINARY_DIR}/test.txt ::/test.txt
        COMMAND rm -f ${CMAKE_BINARY_DIR}/test.txt

        # 5. Write partition image into raw disk at correct offset
        COMMAND echo "[5/5] Writing partition image into disk..."
        COMMAND dd if=${CMAKE_BINARY_DIR}/part.img of=${IDE_DISK_FILE} bs=512 seek=2048 conv=notrunc
        COMMAND rm -f ${CMAKE_BINARY_DIR}/part.img

        COMMAND echo "[DONE] Disk image '${IDE_DISK_FILE}' ready with FAT16 partition and test file."
    )

    add_custom_target(qemu-x86_64-gdb
        COMMAND qemu-system-x86_64 -m 512M -boot d -cdrom ${ISO_FILE} -drive file=${IDE_DISK_FILE},if=ide,index=0,media=disk,format=raw -s -S
        DEPENDS ${KERNEL_TARGET} qemu-x86_64-idedisk
        COMMENT "Debug kernel ISO via QEMU"
    )

    add_custom_target(qemu-x86_64-run
        COMMAND qemu-system-x86_64 -m 512M -boot d -cdrom ${ISO_FILE} -drive file=${IDE_DISK_FILE},if=ide,index=0,media=disk,format=raw
        DEPENDS ${KERNEL_TARGET} qemu-x86_64-idedisk
        COMMENT "Run kernel ISO in QEMU"
    )

endfunction()