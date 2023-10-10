cmake_minimum_required(VERSION 3.20.0)

set(MCU ch32v20x) # specify SDK

set(CMAKE_TOOLCHAIN_FILE ../SDK/${MCU}/toolchain.cmake)
project(blinky C CXX)

add_definitions(
    -D${MCU}
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
    --specs=nano.specs # disable if use exceptions
    --specs=nosys.specs
)

include_directories(
    src/
    src/${MCU}
)

add_subdirectory(../SDK/${MCU} ${CMAKE_CURRENT_BINARY_DIR}/SDK)
add_executable(${PROJECT_NAME}.elf
    src/main.cpp
    src/exception.cpp
    src/${MCU}/dbg.cpp
    src/${MCU}/led.cpp
    src/${MCU}/ticker.cpp
)
target_link_libraries(${PROJECT_NAME}.elf SDK)
