cmake_minimum_required(VERSION 3.20.0)

if (TINYUSB_DRIVER STREQUAL "USBFS")
    set(TINYUSB_DRIVER_SRC wch/dcd_ch32_usbfs.c)
elseif (TINYUSB_DRIVER STREQUAL "USBD")
    set(TINYUSB_DRIVER_SRC st//stm32_fsdev/dcd_stm32_fsdev.c)
else()
    message(WARNING "no driver specified, default to wch/dcd_ch32_usbfs.c")
    set(TINYUSB_DRIVER_SRC wch/dcd_ch32_usbfs.c)
endif()

file(GLOB TINYUSB_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/tinyusb/src/*.c
    ${CMAKE_CURRENT_LIST_DIR}/tinyusb/src/class/**/*.c
    ${CMAKE_CURRENT_LIST_DIR}/tinyusb/src/common/*.c
    ${CMAKE_CURRENT_LIST_DIR}/tinyusb/src/device/*.c
    ${CMAKE_CURRENT_LIST_DIR}/tinyusb/src/portable/${TINYUSB_DRIVER_SRC}
)
add_library(tinyusb ${TINYUSB_SRCS})

target_include_directories(tinyusb PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/tinyusb/src
)

add_subdirectory(../freertos ${CMAKE_CURRENT_BINARY_DIR}/freertos)
target_link_libraries(tinyusb freertos)
