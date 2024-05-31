cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_TOOLCHAIN_FILE ../SDK/attiny1616/toolchain.cmake)
project(word_watch C)

add_compile_definitions(
    F_CPU=20000000L
)

add_compile_options(
    -std=gnu23
    -Wall
    -Werror
    -Os
    -g
    -ffunction-sections
    -fdata-sections
    -ffat-lto-objects
    -flto
)

add_link_options(
    -Wl,--print-memory-usage
    -Wl,--gc-sections
    -Os
    -g
    -flto

    # comment out if not using printf
    -Wl,-u,vfprintf
    -lprintf_min
)

include_directories(
    src
)

add_executable(${PROJECT_NAME}.elf
    src/main.c
    src/dbg.c
)
