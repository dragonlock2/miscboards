cmake_minimum_required(VERSION 3.20.0)

set(MCU lpc1549)
include(../SDK/init.cmake)

project(lpc1549_oaspi)

include_directories(
    inc
)
    
file(GLOB SRCS
    src/*.cpp
)
add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} bootstrap freertos tinyusb)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    # CONFIG_ADIN1110
    CONFIG_NCN26010
    CONFIG_ETH_MIN_LATENCY
)
sdk_finish()
