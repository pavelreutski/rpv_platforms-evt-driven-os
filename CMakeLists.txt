cmake_minimum_required(VERSION 3.20.0)

project(rpv-evt-driven-os
        LANGUAGES NONE
        DESCRIPTION "rpv event driven os")

# set/add solution cmake modules path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/tools/cmake)

include(FetchContent)

# set custom output directories for binaries and libraries

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(CMAKE_AUTOGEN_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/autogen")

# add projects

add_subdirectory(./src/kernel)