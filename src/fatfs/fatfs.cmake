cmake_minimum_required(VERSION 3.20.0)

# Fetch 3rd parties

FetchContent_Declare(
    fatfs
    GIT_REPOSITORY git@github.com:abbrev/fatfs.git
    GIT_TAG        master
)

FetchContent_MakeAvailable(fatfs)

# FatFs options

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

set(FATFS_AUTOGEN_DIR ${CMAKE_AUTOGEN_OUTPUT_DIRECTORY}/fatfs)

configure_file(./fatfs/ffconf.h.in
               ${FATFS_AUTOGEN_DIR}/ffconf.h)

file(GLOB FAT_FS_GLUE_SOURCES CONFIGURE_DEPENDS ./fatfs/*.c)

add_library(fatfs OBJECT ${FAT_FS_GLUE_SOURCES} 
                         ${fatfs_SOURCE_DIR}/source/ff.c
                         ${fatfs_SOURCE_DIR}/source/ffunicode.c)

if(EXISTS ${fatfs_SOURCE_DIR}/source/ffconf.h)
    file(REMOVE ${fatfs_SOURCE_DIR}/source/ffconf.h)
endif()

target_compile_features(fatfs PRIVATE c_std_99)

target_include_directories(fatfs PRIVATE ./include
                                 PUBLIC  ${FATFS_AUTOGEN_DIR}
                                         ${fatfs_SOURCE_DIR}/source)

target_link_libraries(fatfs PRIVATE rpv-kernel)