cmake_minimum_required(VERSION 3.20.0)

set(MCU ch32v20x) # specify SDK

set(CMAKE_TOOLCHAIN_FILE ../SDK/${MCU}/toolchain.cmake)
project(mouse_jiggler ASM C CXX)

add_compile_options(
    $<$<COMPILE_LANGUAGE:C>:-std=gnu18>
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++23>
    -Wall
    -Wextra
    -Werror
    -Wno-error=address # see stm32f1xx.h
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
    src
    src/${MCU}
)

file(GLOB SRCS
    src/*.cpp
    src/${MCU}/*.cpp
)
add_executable(${PROJECT_NAME}.elf ${SRCS})
add_subdirectory(../SDK/${MCU} ${CMAKE_CURRENT_BINARY_DIR}/SDK)
add_subdirectory(../SDK/freertos ${CMAKE_CURRENT_BINARY_DIR}/freertos)
add_subdirectory(../SDK/tinyusb ${CMAKE_CURRENT_BINARY_DIR}/tinyusb)
target_link_libraries(${PROJECT_NAME}.elf SDK freertos tinyusb)
