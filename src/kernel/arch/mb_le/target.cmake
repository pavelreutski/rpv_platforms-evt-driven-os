project(rpv-microblazele-kernel LANGUAGES C ASM)

set(STORAGE_SUBSYS OFF)
set(JOURNAL_SUBSYS OFF)

file(GLOB KERNEL_MBLE_ASM_SOURCES CONFIGURE_DEPENDS ./arch/mb_le/*.S)
file(GLOB_RECURSE KERNEL_MBLE_C_SOURCES CONFIGURE_DEPENDS ./arch/mb_le/*.c)

add_executable(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE c gcc)
target_sources(${PROJECT_NAME} PRIVATE ${KERNEL_MBLE_ASM_SOURCES} ${KERNEL_MBLE_C_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ./include/arch/mb_le)

target_link_options(${PROJECT_NAME} PRIVATE -Wl,-Map,$<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.map
                                            -Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/scripts/kernel-mb_le-linker.ld)