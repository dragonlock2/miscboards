cmake_minimum_required(VERSION 3.20.0)

set(SDK_FREERTOS_TCP_PATH ${CMAKE_CURRENT_LIST_DIR}/../../../shared/FreeRTOS-Plus-TCP)

file(GLOB SDK_FREERTOS_TCP_SRCS
    ${SDK_FREERTOS_TCP_PATH}/source/*.c
    ${SDK_FREERTOS_TCP_PATH}/source/portable/BufferManagement/BufferAllocation_1.c
)
add_library(freertos_tcp ${SDK_FREERTOS_TCP_SRCS})
target_link_libraries(freertos_tcp freertos)
target_include_directories(freertos_tcp SYSTEM PUBLIC
    ${SDK_FREERTOS_TCP_PATH}/source/include
    ${SDK_FREERTOS_TCP_PATH}/source/portable/Compiler/GCC
)
set_target_properties(freertos_tcp PROPERTIES EXCLUDE_FROM_ALL TRUE)

set_source_files_properties( # disable warnings for some files
    ${SDK_FREERTOS_TCP_PATH}/source/FreeRTOS_DNS.c
    ${SDK_FREERTOS_TCP_PATH}/source/FreeRTOS_DNS_Cache.c
    ${SDK_FREERTOS_TCP_PATH}/source/FreeRTOS_Sockets.c
    PROPERTIES APPEND_STRING PROPERTY COMPILE_FLAGS " -w"
)
