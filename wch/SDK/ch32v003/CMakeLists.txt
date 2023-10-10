cmake_minimum_required(VERSION 3.20.0)

add_library(SDK
    ${CMAKE_CURRENT_LIST_DIR}/src/startup.c
    ${CMAKE_CURRENT_LIST_DIR}/src/exception.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/system_ch32v00x.c
    ${WCH_SDK_DIR}/Peripheral/src/ch32v00x_dbgmcu.c
    ${WCH_SDK_DIR}/Peripheral/src/ch32v00x_gpio.c
    ${WCH_SDK_DIR}/Peripheral/src/ch32v00x_misc.c
    ${WCH_SDK_DIR}/Peripheral/src/ch32v00x_rcc.c
    ${WCH_SDK_DIR}/Peripheral/src/ch32v00x_usart.c
)
set_target_properties(SDK PROPERTIES LINKER_LANGUAGE C)

target_include_directories(SDK PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${WCH_SDK_DIR}/Peripheral/inc
    ${WCH_SDK_DIR}/Core
)

set_target_properties(SDK PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})
