cmake_minimum_required(VERSION 3.20.0)

file(GLOB TINYUSB_SRCS
    ../../../shared/tinyusb/src/*.c
    ../../../shared/tinyusb/src/class/**/*.c
    ../../../shared/tinyusb/src/common/*.c
    ../../../shared/tinyusb/src/device/*.c
    ../../../shared/tinyusb/src/portable/nxp/lpc17_40/*.c
    ../../../shared/tinyusb/src/portable/nxp/lpc_ip3511/*.c
)
add_library(tinyusb ${TINYUSB_SRCS})

target_include_directories(tinyusb SYSTEM PUBLIC
    ../../../shared/tinyusb/src
)

target_link_libraries(tinyusb freertos)
