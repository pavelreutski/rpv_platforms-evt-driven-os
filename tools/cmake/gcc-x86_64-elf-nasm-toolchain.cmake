# ================================
# x86_64-elf bare-metal toolchain (C only and NASM)
# ================================

# Target system
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Compiler prefix
set(CROSS x86_64-elf)

# ----------------
# Compiler / tools
# ----------------
set(CMAKE_ASM_NASM_COMPILER nasm)

set(CMAKE_C_COMPILER   ${CROSS}-gcc)
set(CMAKE_LINKER       ${CROSS}-ld)
set(CMAKE_AR           ${CROSS}-ar)
set(CMAKE_GDB          ${CROSS}-gdb)
set(CMAKE_OBJCOPY      ${CROSS}-objcopy)
set(CMAKE_OBJDUMP      ${CROSS}-objdump)
set(CMAKE_RANLIB       ${CROSS}-ranlib)
set(CMAKE_READELF      ${CROSS}-readelf)

# ------------------------------
# Enable ASM_NASM
# ------------------------------
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
set(CMAKE_ASM_NASM_FLAGS "-f elf64 -g -F dwarf")

# ---------------------
# Global compiler flags
# ---------------------

add_compile_options(
    $<$<COMPILE_LANGUAGE:C>:-m64>
    $<$<COMPILE_LANGUAGE:C>:-ffreestanding>
    $<$<COMPILE_LANGUAGE:C>:-fno-pic>
    $<$<COMPILE_LANGUAGE:C>:-fno-pie>
    $<$<COMPILE_LANGUAGE:C>:-fno-omit-frame-pointer>
    $<$<COMPILE_LANGUAGE:C>:-fno-stack-protector>
    $<$<COMPILE_LANGUAGE:C>:-fno-strict-aliasing>
    $<$<COMPILE_LANGUAGE:C>:-O2>
    $<$<COMPILE_LANGUAGE:C>:-g>
    $<$<COMPILE_LANGUAGE:C>:-Wall>
    $<$<COMPILE_LANGUAGE:C>:-Wextra>
)

# ---------------------
# Linker flags
# ---------------------

add_link_options(-nostdlib
                -nostartfiles
                -nodefaultlibs
                -static
                -zmax-page-size=0x1000
                -znoexecstack)

# Prevent CMake from adding system libs
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
