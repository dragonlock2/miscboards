cmake_minimum_required(VERSION 3.30.0)

set(PICO_BOARD pico2)
include(../SDK/init.cmake)

project(sleep)
sdk_init()

include_directories(
    src
)

add_executable(${PROJECT_NAME}
    src/app.cpp
)
target_link_libraries(${PROJECT_NAME}
    freertos
    hardware_sleep
)

pico_enable_stdio_uart(${PROJECT_NAME} 1) # TX=0, RX=1, 115200bps
sdk_finish()
