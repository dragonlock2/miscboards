cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_TOOLCHAIN_FILE ../SDK/ch32v003.cmake)
project(blinky C ASM)

add_compile_options(
    -std=gnu18
    -Wall
    -Werror
    -Os
    -g3
    -ffunction-sections
    -fdata-sections
    -flto
)

add_link_options(
    -Wl,--print-memory-usage
    -Wl,--gc-sections
    -Os
    -g3
    -flto
)

add_executable(${PROJECT_NAME}.elf
    src/main.c
)
target_link_libraries(${PROJECT_NAME}.elf ch32v003)
