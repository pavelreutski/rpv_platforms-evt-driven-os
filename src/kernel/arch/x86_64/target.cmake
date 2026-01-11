project(rpv-x86_64-kernel LANGUAGES C)

include(qemu-x86_64-simul)
include(kernel-x86_64-iso)

include(gcc-x86_64-elf-utils)

# x86_64 system platform
add_subdirectory(./arch/x86_64/sys)

file(GLOB_RECURSE KERNEL_X86_64_C_SOURCES CONFIGURE_DEPENDS ./arch/x86_64/*.c)    

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${KERNEL_X86_64_C_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ./include/arch/x86_64)

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
elf_dump(${PROJECT_NAME})