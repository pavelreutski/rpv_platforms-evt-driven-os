project(rpv-avr-kernel LANGUAGES C)

include(gcc-elf-utils)

include(avr-mcu)
include(avr-mcu_programmer)

# event driven kernel

FetchContent_Declare(
        evtdriven-kernel
        GIT_REPOSITORY git@github.com:pavelreutski/rpv-evt-driven-os.git
        GIT_TAG master
)

FetchContent_MakeAvailable(evtdriven-kernel)

file(GLOB_RECURSE KERNEL_AVR_C_SOURCES CONFIGURE_DEPENDS ./arch/mc_avr/*.c)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${KERNEL_AVR_C_SOURCES})

target_compile_definitions(${PROJECT_NAME} PRIVATE MCU_CONFIG="mcu_config.h")

target_include_directories(${PROJECT_NAME} PRIVATE ./include/mc_avr
                                                   ./include/mc_avr/config)

target_compile_features(${PROJECT_NAME} PRIVATE c_std_23)

target_link_libraries(${PROJECT_NAME} PRIVATE rpv-kernel)
target_link_options(${PROJECT_NAME} PRIVATE -Wl,-Map,$<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.map
                                            -Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/scripts/kernel-mc_avr5-linker.ld)

elf_dump(${PROJECT_NAME})
avr_elf_dump(${PROJECT_NAME})

program_mcu_on_arduino(${PROJECT_NAME} ${MCU_MODEL} ${MCU_UPLOADTOOL_PART})

