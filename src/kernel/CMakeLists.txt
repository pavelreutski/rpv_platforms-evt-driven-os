cmake_minimum_required(VERSION 3.20.0)

option(USE_FATFS "Use fatfs as files I/O backend" ON)

# set(KERNEL_TARGET "x86_64" CACHE STRING "kernel target hardware architecture")
set_property(CACHE KERNEL_TARGET PROPERTY STRINGS "x86_64" "stm32" "microchip_avr")

if(KERNEL_TARGET STREQUAL "x86_64")

    set(USE_FATFS ON)
    include(./arch/x86_64/target.cmake)
elseif(KERNEL_TARGET STREQUAL "microchip_avr")

    set(USE_FATFS OFF)
    include(./arch/microchip_avr/target.cmake)
else()

    get_property(KERNEL_TARGETS CACHE KERNEL_TARGET PROPERTY STRINGS)
    message(FATAL_ERROR "kernel must have hardware architecture target. target: ${KERNEL_TARGET}. supported targets: ${KERNEL_TARGETS}")

endif()

if(USE_FATFS)

    include(./arch/fatfs/fatfs.cmake)
    target_link_libraries(${PROJECT_NAME} PRIVATE fatfs)

endif()

file(GLOB_RECURSE KERNEL_CORE_SOURCES CONFIGURE_DEPENDS ./core/*.c)
file(GLOB_RECURSE KERNEL_UTILS_SOURCES CONFIGURE_DEPENDS ./utils/*.c)

target_compile_features(${PROJECT_NAME} PRIVATE c_std_23)
target_sources(${PROJECT_NAME} PRIVATE ./entry.c
                                       ${KERNEL_CORE_SOURCES}
                                       ${KERNEL_UTILS_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ./include
                                                  ./include/core
                                                  ./include/services)

target_compile_options(${PROJECT_NAME} PRIVATE -mcmodel=kernel)