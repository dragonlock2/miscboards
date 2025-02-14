cmake_minimum_required(VERSION 3.20.0)

set(MCU lpc1549)
include(../SDK/init.cmake)

project(lpc1549_oaspi)

add_compile_definitions(
    # CONFIG_ADIN1110
    CONFIG_NCN26010
)

include_directories(
    inc
)
    
file(GLOB SRCS
    src/*.cpp
)
add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} bootstrap freertos tinyusb)
sdk_finish()
