cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico_w)

# boilerplate
include(../pico-sdk/pico_sdk_init.cmake)
project(keyboard)
pico_sdk_init()

# project specific
add_executable(${PROJECT_NAME}
    src/main.c
    src/kscan.c
    src/ws2812b.c
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror)
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    pico_cyw43_arch_none
    pico_multicore
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
