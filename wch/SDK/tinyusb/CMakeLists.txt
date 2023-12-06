cmake_minimum_required(VERSION 3.20.0)

file(GLOB TINYUSB_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/tinyusb/src/*.c
    ${CMAKE_CURRENT_LIST_DIR}/tinyusb/src/class/**/*.c
    ${CMAKE_CURRENT_LIST_DIR}/tinyusb/src/common/*.c
    ${CMAKE_CURRENT_LIST_DIR}/tinyusb/src/device/*.c
    ${CMAKE_CURRENT_LIST_DIR}/tinyusb/src/portable/wch/dcd_ch32_usbfs.c # TODO pick driver
)
add_library(tinyusb ${TINYUSB_SRCS})

target_include_directories(tinyusb PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/tinyusb/src
)

add_subdirectory(../freertos ${CMAKE_CURRENT_BINARY_DIR}/freertos)
target_link_libraries(tinyusb freertos)
