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
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(${PROJECT_NAME}.elf
    src/main.c
    src/dbg.c
    src/matrix.c
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/matrix_gen.h
    COMMAND python ${CMAKE_CURRENT_LIST_DIR}/lut.py ${CMAKE_CURRENT_BINARY_DIR}/matrix_gen.h
    DEPENDS ${CMAKE_CURRENT_LIST_DIR}/lut.py
)
add_custom_target(matrix_gen ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/matrix_gen.h)
add_dependencies(${PROJECT_NAME}.elf matrix_gen)
