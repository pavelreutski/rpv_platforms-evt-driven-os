cmake_minimum_required(VERSION 3.20.0)

project(rpv-evt-driven-os
        DESCRIPTION "rpv event driven os")

# Set/Add solution cmake modules path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/tools/cmake)

include(qemu-x86_64-gdb)
include(kernel-x86_64-iso)

include(gcc-x86_64-elf-utils)

# Set custom output directories for binaries and libraries

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Add projects

add_subdirectory(./src/kernel)
add_subdirectory(./src/x86_64)