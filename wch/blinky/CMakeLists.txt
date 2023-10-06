cmake_minimum_required(VERSION 3.20.0)

set(MCU ch32v003) # should only need to change this
set(CMAKE_TOOLCHAIN_FILE ../SDK/${MCU}/toolchain.cmake)
project(blinky C ASM)

add_definitions(
    -D${MCU}
)

add_compile_options(
    -std=gnu18
    -Wall
    -Wextra
    -Werror
    -Os
    -g3
    -ffunction-sections
    -fdata-sections
    -ffat-lto-objects
    -flto
)

add_link_options(
    -Wl,--print-memory-usage
    -Wl,--gc-sections
    -Os
    -g3
    -flto
)

add_subdirectory(../SDK/${MCU} ${CMAKE_CURRENT_BINARY_DIR}/SDK)
add_executable(${PROJECT_NAME}.elf
    src/main.c
    src/dbg.c
)
target_link_libraries(${PROJECT_NAME}.elf SDK)
