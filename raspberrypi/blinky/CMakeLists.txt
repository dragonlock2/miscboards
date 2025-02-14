cmake_minimum_required(VERSION 3.30.0)

# set(PICO_PLATFORM rp2350-riscv)
set(PICO_BOARD pico2)
include(../SDK/init.cmake)

project(blinky)
sdk_init()

include_directories(
    src
)

add_executable(${PROJECT_NAME}
    src/app.cpp
)
target_link_libraries(${PROJECT_NAME} freertos)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
sdk_finish()
