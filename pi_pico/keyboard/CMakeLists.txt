cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico_w)

include(../pico-sdk/pico_sdk_init.cmake)
project(keyboard)
pico_sdk_init()

add_executable(keyboard
    src/main.c
    src/kscan.c
)

target_link_libraries(keyboard
    pico_stdlib
    pico_cyw43_arch_none
)

pico_enable_stdio_usb(keyboard 1)
pico_enable_stdio_uart(keyboard 0)

add_custom_command(
    TARGET keyboard POST_BUILD
    COMMAND arm-none-eabi-size keyboard.elf
)
pico_add_extra_outputs(keyboard)

add_custom_target(flash picotool load -f ${PROJECT_NAME}.uf2 DEPENDS keyboard)
