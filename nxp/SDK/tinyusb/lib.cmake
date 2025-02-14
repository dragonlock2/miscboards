cmake_minimum_required(VERSION 3.20.0)

set(SDK_TINYUSB_PATH ${CMAKE_CURRENT_LIST_DIR}/../../../shared/tinyusb)

file(GLOB SDK_TINYUSB_SRCS
    ${SDK_TINYUSB_PATH}/src/*.c
    ${SDK_TINYUSB_PATH}/src/class/**/*.c
    ${SDK_TINYUSB_PATH}/src/common/*.c
    ${SDK_TINYUSB_PATH}/src/device/*.c
    ${SDK_TINYUSB_PATH}/src/portable/nxp/lpc17_40/*.c
    ${SDK_TINYUSB_PATH}/src/portable/nxp/lpc_ip3511/*.c
)
add_library(tinyusb ${SDK_TINYUSB_SRCS})
target_link_libraries(tinyusb freertos)
target_include_directories(tinyusb SYSTEM PUBLIC
    ${SDK_TINYUSB_PATH}/src
)
set_target_properties(tinyusb PROPERTIES EXCLUDE_FROM_ALL TRUE)
