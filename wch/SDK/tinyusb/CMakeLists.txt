cmake_minimum_required(VERSION 3.20.0)

file(GLOB TINYUSB_SRCS
    ../../../shared/tinyusb/src/*.c
    ../../../shared/tinyusb/src/class/**/*.c
    ../../../shared/tinyusb/src/common/*.c
    ../../../shared/tinyusb/src/device/*.c
    ../../../shared/tinyusb/src/portable/wch/dcd_ch32_usbfs.c
    ../../../shared/tinyusb/src/portable/st/stm32_fsdev/dcd_stm32_fsdev.c
)
add_library(tinyusb ${TINYUSB_SRCS})

target_include_directories(tinyusb PUBLIC
    ../../../shared/tinyusb/src
)

target_link_libraries(tinyusb freertos)
