cmake_minimum_required(VERSION 3.30.0)

# set(PICO_PLATFORM rp2350-riscv)
set(PICO_BOARD pico2)
include(../SDK/toolchain.cmake)

project(blinky)
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
