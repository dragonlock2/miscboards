cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico_w)

# boilerplate
include(../pico-sdk/pico_sdk_init.cmake)
project(ble_minimal)
pico_sdk_init()

# project specific
set (CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/ble.cpp
)
target_include_directories(${PROJECT_NAME} PRIVATE
    src/
    ${CMAKE_CURRENT_BINARY_DIR}/generated
)
pico_btstack_make_gatt_header(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/keyboard.gatt)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    PICO_MAX_SHARED_IRQ_HANDLERS=16
    PICO_STACK_SIZE=4096
    PICO_USE_STACK_GUARDS=1
    CYW43_ENABLE_BLUETOOTH=1
    CYW43_LWIP=0
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror)
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    pico_multicore
    pico_btstack_ble
    pico_btstack_cyw43
    pico_cyw43_arch_threadsafe_background
)
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
suppress_btstack_warnings()

# boilerplate
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND arm-none-eabi-size ${PROJECT_NAME}.elf
)
pico_add_extra_outputs(${PROJECT_NAME})
add_custom_target(flash picotool load -f ${PROJECT_NAME}.uf2 DEPENDS ${PROJECT_NAME})
