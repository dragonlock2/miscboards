cmake_minimum_required(VERSION 3.20.0)

file(GLOB FREERTOS_SRCS
    src/${MCU}/main.c
    ../../../shared/FreeRTOS-Kernel/*.c
    ../../../shared/FreeRTOS-Kernel/portable/MemMang/heap_4.c
    ../../../shared/FreeRTOS-Kernel/portable/GCC/RISC-V/port.c
    ../../../shared/FreeRTOS-Kernel/portable/GCC/RISC-V/portASM.S
)
add_library(freertos ${FREERTOS_SRCS})

target_include_directories(freertos PUBLIC
    src/${MCU}
    ../../../shared/FreeRTOS-Kernel/include
    ../../../shared/FreeRTOS-Kernel/portable/GCC/RISC-V
    ../../../shared/FreeRTOS-Kernel/portable/GCC/RISC-V/chip_specific_extensions/RISCV_no_extensions
)

target_link_libraries(freertos SDK)
