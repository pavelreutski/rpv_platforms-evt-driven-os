cmake_minimum_required(VERSION 3.20.0)

project(rpv-x86_64-kernel
        LANGUAGES C ASM_NASM)

file(GLOB_RECURSE KERNEL_C_SOURCES LIST_DIRECTORIES false CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB_RECURSE KERNEL_ASM_SOURCES LIST_DIRECTORIES false CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.asm")

message(STATUS "C sources: ${KERNEL_C_SOURCES}")

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE ${KERNEL_C_SOURCES}
                                       ${KERNEL_ASM_SOURCES})

target_link_options(${PROJECT_NAME} PRIVATE -nostdlib
                                            -T${CMAKE_CURRENT_SOURCE_DIR}/kernel-linker.ld
                                            -Wl,-Map,$<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.map)

elf_dump(${PROJECT_NAME})

kernel_iso(${PROJECT_NAME})
gdb_qemu_iso(${PROJECT_NAME})