set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR microblazeel) # little endian version

set(CROSS_GCC microblaze-elf)

set(CMAKE_C_COMPILER   ${CROSS_GCC}-gcc)
set(CMAKE_LINKER       ${CROSS_GCC}-ld)
set(CMAKE_AR           ${CROSS_GCC}-ar)
set(CMAKE_GDB          ${CROSS_GCC}-gdb)
set(CMAKE_OBJCOPY      ${CROSS_GCC}-objcopy)
set(CMAKE_OBJDUMP      ${CROSS_GCC}-objdump)
set(CMAKE_RANLIB       ${CROSS_GCC}-ranlib)
set(CMAKE_READELF      ${CROSS_GCC}-readelf)

set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

add_link_options(-mcpu=v11.0 -mlittle-endian -Wl,--gc-sections -Wl,--strip-debug -nostdlib -ffreestanding)
add_compile_options(-mcpu=v11.0 -mlittle-endian -mxl-soft-mul
                    -c -O2 -g -gdwarf-4
                    -ffreestanding
                    -fno-inline -fno-builtin -fmerge-constants
                    -fdata-sections -ffunction-sections -fno-common
                    -Wl,--no-relax
                    -Wl,--gc-sections
                    -Wl,--strip-debug
 		            -fshort-enums
		            -Wall -Werror -Wextra
		            -Wl,--start-group
		            -Wl,--end-group
		            -pedantic -pedantic-errors
		            -funsigned-char -funsigned-bitfields)
