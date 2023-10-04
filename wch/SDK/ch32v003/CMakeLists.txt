cmake_minimum_required(VERSION 3.20.0)

add_library(SDK
    ${WCH_SDK_DIR}/Peripheral/src/ch32v00x_gpio.c
    ${WCH_SDK_DIR}/Peripheral/src/ch32v00x_misc.c
    ${WCH_SDK_DIR}/Peripheral/src/ch32v00x_rcc.c
    ${WCH_SDK_DIR}/Peripheral/src/ch32v00x_usart.c

    # TODO remove, replace with custom startup
    ${WCH_SDK_DIR}/Startup/startup_ch32v00x.S
    ${WCH_SDK_DIR}/../GPIO/GPIO_Toggle/User/system_ch32v00x.c
    ${WCH_SDK_DIR}/Debug/debug.c
)
set_target_properties(SDK PROPERTIES LINKER_LANGUAGE C)

target_include_directories(SDK PUBLIC
    ${WCH_SDK_DIR}/Peripheral/inc
    ${WCH_SDK_DIR}/Core
    ${WCH_SDK_DIR}/Debug

    # TODO remove, replace with custom startup
    ${WCH_SDK_DIR}/../GPIO/GPIO_Toggle/User
)
