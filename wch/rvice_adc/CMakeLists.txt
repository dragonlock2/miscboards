cmake_minimum_required(VERSION 3.20.0)

set(MCU ch32v20x) # specify SDK
set(TINYUSB_DRIVER "USBFS")

set(CMAKE_TOOLCHAIN_FILE ../SDK/${MCU}/toolchain.cmake)
project(rvice_adc ASM C CXX)

add_compile_definitions(
    ${MCU}
)

add_compile_options(
    $<$<COMPILE_LANGUAGE:C>:-std=gnu18>
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++23>
    -Wall
    -Wextra
    -Werror
    -O2
    -g3
    -ffunction-sections
    -fdata-sections
    -ffat-lto-objects
    -flto

    # tinyusb
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-enum-enum-conversion>

    # save some flash
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
)

add_link_options(
    -Wl,--print-memory-usage
    -Wl,--gc-sections
    -O2
    -g3
    -flto
    --specs=nano.specs
    --specs=nosys.specs
)

include_directories(
    src
    platform/${MCU}
)

file(GLOB SRCS
    src/*.c
    src/*.cpp
    platform/${MCU}/*.c
    platform/${MCU}/*.cpp
)
add_executable(${PROJECT_NAME}.elf ${SRCS})

add_subdirectory(../SDK/tinyusb ${CMAKE_CURRENT_BINARY_DIR}/tinyusb) # includes SDK and FreeRTOS
target_link_libraries(${PROJECT_NAME}.elf tinyusb)
