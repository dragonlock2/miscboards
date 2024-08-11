cmake_minimum_required(VERSION 3.20.0)

file(GLOB FREERTOS_TCP_SRCS
    ../../../shared/FreeRTOS-Plus-TCP/source/*.c
    ../../../shared/FreeRTOS-Plus-TCP/source/portable/BufferManagement/BufferAllocation_1.c
)
add_library(freertos_tcp ${FREERTOS_TCP_SRCS})

target_include_directories(freertos_tcp SYSTEM PUBLIC
    inc
    inc/${MCU}
    ../../../shared/FreeRTOS-Plus-TCP/source/include
    ../../../shared/FreeRTOS-Plus-TCP/source/portable/Compiler/GCC
)

target_link_libraries(freertos_tcp freertos)

set_source_files_properties( # disable warnings for some files
    ../../../shared/FreeRTOS-Plus-TCP/source/FreeRTOS_DNS.c
    ../../../shared/FreeRTOS-Plus-TCP/source/FreeRTOS_Routing.c
    ../../../shared/FreeRTOS-Plus-TCP/source/FreeRTOS_Sockets.c
    PROPERTIES APPEND_STRING PROPERTY COMPILE_FLAGS " -w"
)
