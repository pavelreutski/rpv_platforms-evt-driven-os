# ================================
# x86_64-elf bare-metal toolchain (C only and NASM)
# ================================

# ----------------
# Locate x86_64-elf-gcc in PATH
# ----------------

find_program(CROSS_GCC x86_64-elf-gcc)

get_filename_component(CROSS_BIN_DIR "${CROSS_GCC}" DIRECTORY)   # .../bin
get_filename_component(CROSS_PREFIX "${CROSS_BIN_DIR}" DIRECTORY) # .../ (prefix)

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

set(C_FLAGS -m64 -Wall -Wextra -O2 -g -gdwarf-4 
            -ffreestanding -fno-builtin
            -mno-red-zone -mno-sse -mno-sse2 -mno-mmx -msoft-float
            -fno-pic -fno-pie -fno-omit-frame-pointer -fno-stack-protector -fno-strict-aliasing)

add_compile_options(
    "$<$<COMPILE_LANGUAGE:C>:${C_FLAGS}>")

# ---------------------
# Include directories
# ---------------------
include_directories("${CROSS_PREFIX}/include")

# Prevent CMake from adding system libs
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
