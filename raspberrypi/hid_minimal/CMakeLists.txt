cmake_minimum_required(VERSION 3.30.0)

set(PICO_BOARD pico_w)
include(../SDK/init.cmake)

project(hid_minimal)
set(TINYUSB_OPT_OS OPT_OS_FREERTOS) # USB
sdk_init()

include_directories(
    inc
    ${CMAKE_CURRENT_BINARY_DIR}/generated # BLE
)

add_executable(${PROJECT_NAME}
    src/app.cpp
    src/ble.cpp
    src/usb.cpp
)
target_link_libraries(${PROJECT_NAME}
    freertos
    pico_btstack_ble # BLE
    pico_btstack_cyw43 # BLE
    tinyusb_device # USB
)

pico_btstack_make_gatt_header(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/hid.gatt) # BLE

pico_enable_stdio_rtt(${PROJECT_NAME} 1) # use JLinkRTTViewer
sdk_finish()
