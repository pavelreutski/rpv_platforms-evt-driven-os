project(rpv-avr-kernel LANGUAGES C)

set(SHELL_COMMAND_ARGS          2)
set(SHELL_COMMAND_BUFFER        15)
set(SHELL_COMMANDS_STACK        1)

set(STORAGE_SUBSYS              OFF)
set(JOURNAL_SUBSYS              OFF)

include(avr-mcu)
include(avr-mcu_programmer)

file(GLOB_RECURSE KERNEL_AVR_C_SOURCES CONFIGURE_DEPENDS ./arch/mc_avr/*.c)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${KERNEL_AVR_C_SOURCES})

target_compile_definitions(${PROJECT_NAME} PRIVATE MCU_CONFIG="mcu_config.h")

target_include_directories(${PROJECT_NAME} PRIVATE ./include/arch/mc_avr
                                                   ./include/arch/mc_avr/config)

target_link_options(${PROJECT_NAME} PRIVATE -Wl,-Map,$<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.map
                                            -Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/scripts/kernel-mc_avr5-linker.ld)

avr_elf_dump(${PROJECT_NAME})
program_mcu_on_arduino(${PROJECT_NAME} ${MCU_MODEL} ${MCU_UPLOADTOOL_PART})

