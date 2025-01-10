cmake_minimum_required(VERSION 3.30.0)

set(PICO_BOARD pico_w)
include(../SDK/toolchain.cmake)

project(ble_minimal)
pico_sdk_init()

include_directories(
    src
)

add_executable(${PROJECT_NAME}
    src/app.cpp
)
add_subdirectory(../SDK/freertos ${CMAKE_CURRENT_BINARY_DIR}/freertos)
target_link_libraries(${PROJECT_NAME} freertos)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_finalize()
