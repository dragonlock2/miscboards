cmake_minimum_required(VERSION 3.20.0)

set(MCU ch32v20x) # specify SDK

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
    platform/${MCU}/rtos
    ../SDK/FreeRTOS-Kernel/include
    ../SDK/FreeRTOS-Kernel/portable/GCC/RISC-V
    ../SDK/FreeRTOS-Kernel/portable/GCC/RISC-V/chip_specific_extensions/RISCV_no_extensions
)

file(GLOB SRCS
    src/*.cpp
    platform/${MCU}/*.cpp
    platform/${MCU}/rtos/*.c
    ../SDK/FreeRTOS-Kernel/*.c
    ../SDK/FreeRTOS-Kernel/portable/MemMang/heap_4.c
    ../SDK/FreeRTOS-Kernel/portable/GCC/RISC-V/port.c
    ../SDK/FreeRTOS-Kernel/portable/GCC/RISC-V/portASM.S
)
add_executable(${PROJECT_NAME}.elf ${SRCS})
add_subdirectory(../SDK/${MCU} ${CMAKE_CURRENT_BINARY_DIR}/SDK)
target_link_libraries(${PROJECT_NAME}.elf SDK)
