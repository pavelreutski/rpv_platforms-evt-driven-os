project(rpv-avr-kernel LANGUAGES C)

include(avr-mcu)

file(GLOB_RECURSE KERNEL_AVR_C_SOURCES CONFIGURE_DEPENDS ./arch/microchip_avr/*.c)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${KERNEL_AVR_C_SOURCES})

target_link_options(${PROJECT_NAME} PRIVATE -Wl,-Map,$<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.map)