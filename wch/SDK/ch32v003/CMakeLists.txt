cmake_minimum_required(VERSION 3.20.0)

file(GLOB PERIPHERAL_SRCS ${WCH_SDK_DIR}/Peripheral/src/*.c)
add_library(SDK
    ${CMAKE_CURRENT_LIST_DIR}/src/startup.c
    ${CMAKE_CURRENT_LIST_DIR}/src/system_ch32v00x.c
    ${PERIPHERAL_SRCS}
)
set_target_properties(SDK PROPERTIES LINKER_LANGUAGE C)

target_include_directories(SDK PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${WCH_SDK_DIR}/Peripheral/inc
    ${WCH_SDK_DIR}/Core
)

set_target_properties(SDK PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})
