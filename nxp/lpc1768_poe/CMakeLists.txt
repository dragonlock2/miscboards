cmake_minimum_required(VERSION 3.20.0)

set(MCU lpc1768)
include(../SDK/init.cmake)

project(lpc1768_poe)

include_directories(
    inc
    ../../shared/mongoose
)

file(GLOB SRCS
    src/*.cpp
    ../../shared/mongoose/mongoose.c
)
add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} bootstrap freertos freertos_tcp tinyusb)
sdk_finish()
