cmake_minimum_required(VERSION 3.20.0)

# --- FatFs options ---

set(FF_FS_READONLY      0)
set(FF_USE_LFN          1)
set(FF_MULTI_PARTITION  0)

# --- Features ---

set(FF_FS_MINIMIZE      0)
set(FF_USE_STRFUNC      0)
set(FF_USE_FIND         1)
set(FF_USE_MKFS         1)
set(FF_USE_FASTSEEK     1)
set(FF_USE_EXPAND       0)
set(FF_USE_CHMOD        0)
set(FF_USE_LABEL        1)
set(FF_USE_FORWARD      0)

# --- Locale & Strings ---

set(FF_CODE_PAGE        866)
set(FF_MAX_LFN          255)
set(FF_LFN_UNICODE      0)
set(FF_LFN_BUF          255)
set(FF_SFN_BUF          12)

# --- Filesystem limits ---

set(FF_FS_RPATH         2)
set(FF_VOLUMES          4)
set(FF_STR_VOLUME_ID    1)
set(FF_MIN_SS           512)
set(FF_MAX_SS           4096)

# --- System ---

set(FF_FS_TINY          0)
set(FF_FS_NORTC         1)
set(FF_FS_NOFSINFO      0)
set(FF_FS_LOCK          0)
set(FF_FS_REENTRANT     0)
set(FF_FS_TIMEOUT       1000)

# FatFs vendored directory

set(FATFS_DIR
    "${PROJECT_SOURCE_DIR}/fatfs"
)

set(FATFS_UPSTREAM_DIR
    "${FATFS_DIR}/upstream")

# --- Configuration generation ---

if(NOT CMAKE_AUTOGEN_OUTPUT_DIRECTORY)
    set(CMAKE_AUTOGEN_OUTPUT_DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}/autogen"
    )
endif()

set(FATFS_AUTOGEN_DIR
    "${CMAKE_AUTOGEN_OUTPUT_DIRECTORY}/fatfs"
)

file(MAKE_DIRECTORY "${FATFS_AUTOGEN_DIR}")

configure_file(
    "${FATFS_DIR}/config/ffconf.h.in"
    "${FATFS_AUTOGEN_DIR}/ffconf.h"
)

# --- RPV evt driven kernel file subsystem bridge sources ---

file(GLOB FAT_FS_BRIDGE
    CONFIGURE_DEPENDS
    "${FATFS_DIR}/bridge/*.c"
)

# --- FatFs library ---

add_library(fatfs OBJECT
    ${FAT_FS_BRIDGE}
    "${FATFS_UPSTREAM_DIR}/source/ff.c"
    "${FATFS_UPSTREAM_DIR}/source/ffunicode.c"
)

target_compile_features(fatfs PRIVATE c_std_99)

target_include_directories(fatfs
    PRIVATE 
        "${PROJECT_SOURCE_DIR}/include"
    PUBLIC
        "${FATFS_AUTOGEN_DIR}"
        "${FATFS_UPSTREAM_DIR}/source"
)

target_link_libraries(fatfs PRIVATE rpv-kernel)