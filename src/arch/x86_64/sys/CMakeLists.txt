cmake_minimum_required(VERSION 3.20.0)

project(rpv-x86_64-bootstrap LANGUAGES ASM_NASM)

file(GLOB_RECURSE BOOTSTRAP_ASM_SOURCES CONFIGURE_DEPENDS ./*.asm)

add_library(${PROJECT_NAME} OBJECT)

target_sources(${PROJECT_NAME} PRIVATE ${BOOTSTRAP_ASM_SOURCES})
target_compile_options(${PROJECT_NAME} PRIVATE -f elf64 -g -F dwarf)