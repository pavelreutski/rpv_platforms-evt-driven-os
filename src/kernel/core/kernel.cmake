cmake_minimum_required(VERSION 3.20.0)

set(SHELL_COMMAND_ARGS      10)
set(SHELL_COMMAND_BUFFER    255)

set(SHELL_COMMANDS_STACK    5)

set(STORAGE_SUBSYS          ON)
set(JOURNAL_SUBSYS          ON)

file(GLOB KERNEL_CORE_C_SOURCES CONFIGURE_DEPENDS ./core/kernel.c
                                                  ./core/kernel_cmd.c
                                                  ./core/kernel_mon.c
                                                  ./core/kernel_shell.c
                                                  ./core/kernel_stdio.c)

file(GLOB KERNEL_GENERAL_COMMAND_C_SOURCES CONFIGURE_DEPENDS ./core/commands/*.c)

file(GLOB KERNEL_JOURNAL_SUBSYS_C_SOURCES CONFIGURE_DEPENDS ./core/kernel_jrnl.c
                                                            ./core/commands/journal/*.c)

file(GLOB KERNEL_STORAGE_SUBSYS_C_SOURCES CONFIGURE_DEPENDS ./utils/*.c
                                                            ./core/kernel_fsh.c
                                                            ./core/kernel_fio.c
                                                            ./core/commands/storage/*.c)