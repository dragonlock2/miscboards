cmake_minimum_required(VERSION 3.13)

# boilerplate
include(../pico-sdk/pico_sdk_init.cmake)
project(keyboard)
pico_sdk_init()

# actual stuffs
add_executable(keyboard
    src/main.c
)

target_link_libraries(keyboard pico_stdlib)

# boilerplate
add_custom_command(
    TARGET keyboard POST_BUILD
    COMMAND arm-none-eabi-size keyboard.elf
)
pico_add_extra_outputs(keyboard)

# TODO add flashing target
