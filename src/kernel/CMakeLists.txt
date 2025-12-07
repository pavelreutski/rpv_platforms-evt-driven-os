cmake_minimum_required(VERSION 3.20.0)

project(rpv-x86_64-kernel 
        LANGUAGES C)

file(GLOB_RECURSE KERNEL_C_SOURCES LIST_DIRECTORIES false CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE ${KERNEL_C_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ./include)

target_link_libraries(${PROJECT_NAME} PRIVATE rpv-x86_64-bootstrap)
target_link_options(${PROJECT_NAME} PRIVATE -T${CMAKE_CURRENT_SOURCE_DIR}/scripts/kernel-x86_64-linker.ld
                                            -Wl,-Map,$<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.map)

elf_dump(${PROJECT_NAME})

kernel_iso(${PROJECT_NAME})
gdb_qemu_iso(${PROJECT_NAME})