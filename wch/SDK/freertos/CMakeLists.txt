cmake_minimum_required(VERSION 3.20.0)

file(GLOB FREERTOS_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/src/${MCU}/main.c
    ${CMAKE_CURRENT_LIST_DIR}/FreeRTOS-Kernel/*.c
    ${CMAKE_CURRENT_LIST_DIR}/FreeRTOS-Kernel/portable/MemMang/heap_4.c
    ${CMAKE_CURRENT_LIST_DIR}/FreeRTOS-Kernel/portable/GCC/RISC-V/port.c
    ${CMAKE_CURRENT_LIST_DIR}/FreeRTOS-Kernel/portable/GCC/RISC-V/portASM.S
)
add_library(freertos ${FREERTOS_SRCS})

target_include_directories(freertos PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src/${MCU}
    ${CMAKE_CURRENT_LIST_DIR}/FreeRTOS-Kernel/include
    ${CMAKE_CURRENT_LIST_DIR}/FreeRTOS-Kernel/portable/GCC/RISC-V
    ${CMAKE_CURRENT_LIST_DIR}/FreeRTOS-Kernel/portable/GCC/RISC-V/chip_specific_extensions/RISCV_no_extensions
)

add_subdirectory(../${MCU} ${CMAKE_CURRENT_BINARY_DIR}/SDK)
target_link_libraries(freertos SDK)
