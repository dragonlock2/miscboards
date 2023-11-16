cmake_minimum_required(VERSION 3.20.0)

set(MCU ch32v003) # specify SDK

set(CMAKE_TOOLCHAIN_FILE ../SDK/${MCU}/toolchain.cmake)
project(blinky C CXX)

add_compile_definitions(
    ${MCU}
)

add_compile_options(
    $<$<COMPILE_LANGUAGE:C>:-std=gnu18>
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++23>
    -Wall
    -Wextra
    -Werror
    -Os
    -g3
    -ffunction-sections
    -fdata-sections
    -ffat-lto-objects
    -flto

    # save some flash
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
)

add_link_options(
    -Wl,--print-memory-usage
    -Wl,--gc-sections
    -Os
    -g3
    -flto
    --specs=nano.specs
    --specs=nosys.specs
)

include_directories(
    src/
    src/${MCU}
)

file(GLOB SRCS
    src/*.cpp
    src/${MCU}/*.cpp
)
add_executable(${PROJECT_NAME}.elf ${SRCS})
add_subdirectory(../SDK/${MCU} ${CMAKE_CURRENT_BINARY_DIR}/SDK)
target_link_libraries(${PROJECT_NAME}.elf SDK)
