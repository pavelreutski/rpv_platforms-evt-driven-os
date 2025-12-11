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

# ---------------------
# Global compiler flags
# ---------------------

add_compile_options(
    "$<$<COMPILE_LANGUAGE:C>:-m64;-Wall;-Wextra>")

# Prevent CMake from adding system libs
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
