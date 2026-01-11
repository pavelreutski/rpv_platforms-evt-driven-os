project(rpv-avr-kernel LANGUAGES C)

file(GLOB_RECURSE KERNEL_AVR_C_SOURCES CONFIGURE_DEPENDS ./arch/microchip_avr/*.c)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${KERNEL_AVR_C_SOURCES})