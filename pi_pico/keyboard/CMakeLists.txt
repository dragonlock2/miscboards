cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico_w)

# boilerplate
include(../pico-sdk/pico_sdk_init.cmake)
project(keyboard)
pico_sdk_init()

# project specific
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/kscan.cpp
    src/ws2812b.cpp
    src/ssd1306.cpp
    src/gui.cpp
)
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/src/ws2812b.pio OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

add_compile_definitions(
    PICO_MAX_SHARED_IRQ_HANDLERS=16
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror)
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    pico_cyw43_arch_none
    pico_multicore
    hardware_pio
    hardware_i2c
)
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# boilerplate
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND arm-none-eabi-size ${PROJECT_NAME}.elf
)
pico_add_extra_outputs(${PROJECT_NAME})
add_custom_target(flash picotool load -f ${PROJECT_NAME}.uf2 DEPENDS ${PROJECT_NAME})
