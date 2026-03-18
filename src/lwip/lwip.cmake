cmake_minimum_required(VERSION 3.20.0)

FetchContent_Declare(
    lwip
    GIT_REPOSITORY https://git.savannah.nongnu.org/git/lwip.git
    GIT_TAG STABLE-2_1_3_RELEASE
    GIT_SHALLOW TRUE
)

FetchContent_GetProperties(lwip)

if(NOT lwip_POPULATED)
    FetchContent_Populate(lwip)
endif()

set(LWIP_PBUF_POOL_SIZE 16 CACHE STRING "Number of pbufs")
set(LWIP_PBUF_POOL_BUFSIZE 1536 CACHE STRING "Size of each pbuf")

set(LWIP_DIR ${lwip_SOURCE_DIR})

set(LWIP_AUTOGEN_DIR ${CMAKE_AUTOGEN_OUTPUT_DIRECTORY}/lwip)

configure_file(
    ./lwip/lwipopts.h.in
    ${LWIP_AUTOGEN_DIR}/lwipopts.h)

file(GLOB LWIP_CORE CONFIGURE_DEPENDS
    ${LWIP_DIR}/src/core/*.c
)

file(GLOB LWIP_IPV4 CONFIGURE_DEPENDS
    ${LWIP_DIR}/src/core/ipv4/*.c
)

file(GLOB LWIP_NETIF CONFIGURE_DEPENDS
    ${LWIP_DIR}/src/netif/*.c
)

file(GLOB LWIP_GLUE_SOURCES CONFIGURE_DEPENDS ./lwip/*.c)

add_library(lwip OBJECT ${LWIP_CORE}
                        ${LWIP_IPV4}
                        ${LWIP_NETIF}
                        ${LWIP_GLUE_SOURCES})

target_compile_features(lwip PRIVATE c_std_99)

target_include_directories(lwip PRIVATE ./include
                                PUBLIC ./lwip
                                       ${LWIP_AUTOGEN_DIR}
                                       ${LWIP_DIR}/src/include)


