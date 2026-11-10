cmake_minimum_required(VERSION 3.20.0)

set(LWIP_PBUF_POOL_SIZE 16 CACHE STRING "Number of pbufs")
set(LWIP_PBUF_POOL_BUFSIZE 1536 CACHE STRING "Size of each pbuf")

if(NOT CMAKE_AUTOGEN_OUTPUT_DIRECTORY)
    set(CMAKE_AUTOGEN_OUTPUT_DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}/autogen"
    )
endif()

set(LWIP_DIR ${PROJECT_SOURCE_DIR}/lwip)
set(LWIP_UPSTREAM_DIR ${LWIP_DIR}/upstream)
set(LWIP_AUTOGEN_DIR ${CMAKE_AUTOGEN_OUTPUT_DIRECTORY}/lwip)

configure_file(
    ${LWIP_DIR}/config/lwipopts.h.in
    ${LWIP_AUTOGEN_DIR}/lwipopts.h)

file(GLOB LWIP_CORE CONFIGURE_DEPENDS
    ${LWIP_UPSTREAM_DIR}/src/core/*.c
)

file(GLOB LWIP_IPV4 CONFIGURE_DEPENDS
    ${LWIP_UPSTREAM_DIR}/src/core/ipv4/*.c
)

file(GLOB LWIP_NETIF CONFIGURE_DEPENDS
    ${LWIP_UPSTREAM_DIR}/src/netif/*.c
)

file(GLOB LWIP_BRIDGE CONFIGURE_DEPENDS 
    ${LWIP_DIR}/bridge/*.c
)

add_library(lwip OBJECT ${LWIP_CORE}
                        ${LWIP_IPV4}
                        ${LWIP_NETIF}
                        ${LWIP_BRIDGE})

target_compile_features(lwip PRIVATE c_std_99)

target_include_directories(lwip PRIVATE ${PROJECT_SOURCE_DIR}/include
                                PUBLIC ${LWIP_DIR}
                                       ${LWIP_AUTOGEN_DIR}
                                       ${LWIP_UPSTREAM_DIR}/src/include)