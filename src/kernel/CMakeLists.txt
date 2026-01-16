cmake_minimum_required(VERSION 3.20.0)

include(gcc-elf-utils)

# set(KERNEL_TARGET "x86_64" CACHE STRING "kernel target hardware architecture")
set_property(CACHE KERNEL_TARGET PROPERTY STRINGS "x86_64" "stm32" "microchip_avr")

include(./core/kernel.cmake)

if(KERNEL_TARGET STREQUAL "x86_64")

    include(./arch/x86_64/target.cmake)
elseif(KERNEL_TARGET STREQUAL "microchip_avr")

    include(./arch/mc_avr/target.cmake)
else()

    get_property(KERNEL_TARGETS CACHE KERNEL_TARGET PROPERTY STRINGS)
    message(FATAL_ERROR "kernel must have hardware architecture target. target: ${KERNEL_TARGET}. supported targets: ${KERNEL_TARGETS}")

endif()

set(KERNEL_AUTOGEN_DIR ${CMAKE_AUTOGEN_OUTPUT_DIRECTORY}/kernel)

configure_file(./include/core/kernel_conf.h.in
               ${KERNEL_AUTOGEN_DIR}/kernel_conf.h)

if (JOURNAL_SUBSYS)
    target_sources(${PROJECT_NAME} PRIVATE ${KERNEL_JOURNAL_SUBSYS_C_SOURCES})
endif()

if(STORAGE_SUBSYS)

    if(JOURNAL_SUBSYS)
        target_sources(${PROJECT_NAME} PRIVATE ${KERNEL_STORAGE_SUBSYS_C_SOURCES})
    else()
        message(FATAL_ERROR "wrong subsys configuration: kernel storage subsys is dependant on journal one")
    endif()

endif()

target_compile_features(${PROJECT_NAME} PRIVATE c_std_23)
target_sources(${PROJECT_NAME} PRIVATE ${KERNEL_CORE_C_SOURCES}
                                       ${KERNEL_GENERAL_COMMAND_C_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ./include
                                                  ./include/core
                                                  ./include/services
                                                  ${KERNEL_AUTOGEN_DIR})

elf_dump(${PROJECT_NAME})