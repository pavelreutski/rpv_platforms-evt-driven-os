cmake_minimum_required(VERSION 3.20.0)

option(USE_FATFS "Use fatfs as files I/O backend" ON)

set(KERNEL_TARGET "x86_64" CACHE STRING "kernel target hardware architecture")
set_property(CACHE KERNEL_TARGET PROPERTY STRINGS "x86_64" "stm32" "atmega")

if(KERNEL_TARGET STREQUAL "x86_64")

    project(rpv-x86_64-kernel LANGUAGES C)
    file(GLOB_RECURSE KERNEL_X86_C_SOURCES CONFIGURE_DEPENDS ./arch/x86/*.c)

    add_executable(${PROJECT_NAME})
    target_sources(${PROJECT_NAME} PRIVATE ${KERNEL_X86_C_SOURCES})

    target_include_directories(${PROJECT_NAME} PRIVATE ./include/arch/x86)

    target_link_libraries(${PROJECT_NAME} PRIVATE rpv-x86_64-bootstrap)

    target_link_options(${PROJECT_NAME} PRIVATE -nostdlib
                                            -nostartfiles
                                            -static
                                            -zmax-page-size=0x1000
                                            -znoexecstack
                                            -T${CMAKE_CURRENT_SOURCE_DIR}/scripts/kernel-x86_64-linker.ld
                                            -Wl,-Map,$<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.map)

    kernel_iso(${PROJECT_NAME})
    qemu_simul(${PROJECT_NAME})

else()

    get_property(KERNEL_TARGETS CACHE KERNEL_TARGET PROPERTY STRINGS)
    message(FATAL_ERROR "kernel must have hardware architecture target. supported targets: ${KERNEL_TARGETS}")

endif()

if(USE_FATFS)

    include(./arch/fatfs/fatfs.cmake)
    target_link_libraries(${PROJECT_NAME} PRIVATE fatfs)

endif()

file(GLOB_RECURSE KERNEL_CORE_SOURCES CONFIGURE_DEPENDS ./core/*.c)

target_compile_features(${PROJECT_NAME} PRIVATE c_std_23)
target_sources(${PROJECT_NAME} PRIVATE ./entry.c
                                       ${KERNEL_CORE_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ./include
                                                  ./include/core
                                                  ./include/services)

target_compile_options(${PROJECT_NAME} PRIVATE -mcmodel=kernel)

elf_dump(${PROJECT_NAME})