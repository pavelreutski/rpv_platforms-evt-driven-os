cmake_minimum_required(VERSION 3.20.0)

include(gcc-elf-utils)

set_property(CACHE KERNEL_TARGET PROPERTY STRINGS "x86_64" "stm32" "microchip_avr" "microblaze_le")

if(KERNEL_TARGET STREQUAL "x86_64")
    include(./arch/x86_64/target.cmake)
elseif(KERNEL_TARGET STREQUAL "microchip_avr")
    include(./arch/mc_avr/target.cmake)
elseif(KERNEL_TARGET STREQUAL "microblaze_le")
    include(./arch/mb_le/target.cmake)
else()

    get_property(KERNEL_TARGETS CACHE KERNEL_TARGET PROPERTY STRINGS)
    message(FATAL_ERROR "kernel must have hardware architecture target. target: ${KERNEL_TARGET}. supported targets: ${KERNEL_TARGETS}")

endif()

# event driven kernel

FetchContent_Declare(
        evtdriven-kernel
        SOURCE_DIR ../../../../rpv-evt-driven-kernel
)

FetchContent_MakeAvailable(evtdriven-kernel)

target_compile_features(${PROJECT_NAME} PRIVATE c_std_23)

elf_dump(${PROJECT_NAME})