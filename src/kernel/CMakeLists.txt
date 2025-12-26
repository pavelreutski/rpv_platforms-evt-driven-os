cmake_minimum_required(VERSION 3.20.0)

project(rpv-x86_64-kernel LANGUAGES C)

include(./arch/fatfs/fatfs.cmake)

file(GLOB_RECURSE KERNEL_C_SOURCES CONFIGURE_DEPENDS ./*.c)

add_executable(${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} PRIVATE c_std_23)
target_sources(${PROJECT_NAME} PRIVATE ${KERNEL_C_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ./include
                                                  ./include/core
                                                  ./include/services
                                                  ./include/arch/x86)

target_link_libraries(${PROJECT_NAME} PRIVATE rpv-x86_64-bootstrap fatfs)

target_compile_options(${PROJECT_NAME} PRIVATE -mcmodel=kernel)

target_link_options(${PROJECT_NAME} PRIVATE -nostdlib
                                            -nostartfiles
                                            -static
                                            -zmax-page-size=0x1000
                                            -znoexecstack
                                            -T${CMAKE_CURRENT_SOURCE_DIR}/scripts/kernel-x86_64-linker.ld
                                            -Wl,-Map,$<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.map)

elf_dump(${PROJECT_NAME})

kernel_iso(${PROJECT_NAME})
qemu_simul(${PROJECT_NAME})