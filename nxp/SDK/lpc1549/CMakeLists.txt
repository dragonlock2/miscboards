cmake_minimum_required(VERSION 3.20.0)

# sdk copied from LPCXpresso LPC1549 board v2.20, modified to fix some warnings
# https://www.nxp.com/design/design-center/software/software-library/lpcopen-software-development-platform-lpc15xx:LPCOPEN-SOFTWARE-FOR-LPC15XX

file(GLOB SDK_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/startup.c
    ${CMAKE_CURRENT_LIST_DIR}/src/*.c
)
add_library(SDK ${SDK_SRCS})

target_include_directories(SDK SYSTEM PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/inc
)
