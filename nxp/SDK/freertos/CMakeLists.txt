cmake_minimum_required(VERSION 3.20.0)

file(GLOB FREERTOS_SRCS
    src/main.c
    ../../../shared/FreeRTOS-Kernel/*.c
    ../../../shared/FreeRTOS-Kernel/portable/MemMang/heap_4.c
    ../../../shared/FreeRTOS-Kernel/portable/GCC/ARM_CM3/port.c
)
add_library(freertos ${FREERTOS_SRCS})

target_include_directories(freertos SYSTEM PUBLIC
    inc/${MCU}
    ../../../shared/FreeRTOS-Kernel/include
    ../../../shared/FreeRTOS-Kernel/portable/GCC/ARM_CM3
)

target_link_libraries(freertos SDK)
