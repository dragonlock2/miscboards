cmake_minimum_required(VERSION 3.20.0)

set(SDK_FREERTOS_PATH ${CMAKE_CURRENT_LIST_DIR}/../../../shared/FreeRTOS-Kernel)

file(GLOB SDK_FREERTOS_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${SDK_FREERTOS_PATH}/*.c
    ${SDK_FREERTOS_PATH}/portable/MemMang/heap_4.c
    ${SDK_FREERTOS_PATH}/portable/GCC/ARM_CM3/port.c
)
add_library(freertos ${SDK_FREERTOS_SRCS})
target_link_libraries(freertos bootstrap)
target_include_directories(freertos SYSTEM PUBLIC
    ${SDK_FREERTOS_PATH}/include
    ${SDK_FREERTOS_PATH}/portable/GCC/ARM_CM3
)
set_target_properties(freertos PROPERTIES EXCLUDE_FROM_ALL TRUE)
