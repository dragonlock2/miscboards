cmake_minimum_required(VERSION 3.20.0)

set(MCU lpc1768)
set(CMAKE_TOOLCHAIN_FILE ../SDK/${MCU}/toolchain.cmake)

project(lpc1768_poe C CXX ASM)

add_compile_options(
    $<$<COMPILE_LANGUAGE:C>:-std=gnu18>
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++23>
    -Wall
    -Wextra
    -Werror
    -O3
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
    -O3
    -g3
    -flto=auto
    --specs=nano.specs # comment if using exceptions, need to increase stack size
    --specs=nosys.specs
)

include_directories(
    inc
    ../../shared/mongoose
)

file(GLOB SRCS
    src/*.cpp
    ../../shared/mongoose/mongoose.c
)
add_executable(${PROJECT_NAME}.elf ${SRCS})
add_subdirectory(../SDK/${MCU} ${CMAKE_CURRENT_BINARY_DIR}/SDK)
add_subdirectory(../SDK/freertos ${CMAKE_CURRENT_BINARY_DIR}/freertos)
add_subdirectory(../SDK/freertos_tcp ${CMAKE_CURRENT_BINARY_DIR}/freertos_tcp)
add_subdirectory(../SDK/tinyusb ${CMAKE_CURRENT_BINARY_DIR}/tinyusb)
target_link_libraries(${PROJECT_NAME}.elf SDK freertos freertos_tcp tinyusb)
