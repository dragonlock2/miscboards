cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_SYSTEM_NAME  Generic)
set(CMAKE_C_COMPILER   arm-none-eabi-gcc) # tested on 13.2.rel1
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_OBJCOPY      arm-none-eabi-objcopy)

set(CMAKE_C_COMPILER_WORKS   1)
set(CMAKE_CXX_COMPILER_WORKS 1)

project(lpc1768_poe C CXX ASM)

# sdk copied from LPCXpresso LPC1769 board v2.10
# https://www.nxp.com/design/design-center/software/software-library/lpcopen-software-development-platform-lpc17xx

add_compile_definitions(
    CORE_M3
)

add_compile_options(
    -mcpu=cortex-m3
    $<$<COMPILE_LANGUAGE:C>:-std=gnu18>
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++23>
    -Wall
    -Wextra
    -Werror
    -Wno-unused-parameter # lots in sdk
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
    -mcpu=cortex-m3
    -T${CMAKE_CURRENT_LIST_DIR}/platform/linker.ld
    -Wl,--print-memory-usage
    -Wl,--gc-sections
    -Os
    -g3
    -flto
    -nostartfiles
    --specs=nano.specs
    --specs=nosys.specs
)

file(GLOB SRCS
    src/main.c
    platform/startup.c
)
add_executable(${PROJECT_NAME}.elf ${SRCS})

include_directories(
    src
    platform/inc
)

add_custom_target(${PROJECT_NAME}.hex ALL ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex DEPENDS ${PROJECT_NAME}.elf)
add_custom_target(${PROJECT_NAME}.txt ALL ${CMAKE_OBJDUMP} -d -j .text -j .data -j .bss ${PROJECT_NAME}.elf > ${PROJECT_NAME}.txt DEPENDS ${PROJECT_NAME}.elf)
set_property(TARGET ${PROJECT_NAME}.hex APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${PROJECT_NAME}.hex)
set_property(TARGET ${PROJECT_NAME}.txt APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${PROJECT_NAME}.txt)

add_custom_target(flash  pyocd load ${PROJECT_NAME}.elf -t lpc1768 DEPENDS ${PROJECT_NAME}.elf)
add_custom_target(server pyocd gdbserver --persist -f 4MHz -t lpc1768 DEPENDS ${PROJECT_NAME}.elf)
add_custom_target(gdb    arm-none-eabi-gdb ${PROJECT_NAME}.elf -ex "tar ext localhost:3333" -ex "load" -ex "mon reset halt" DEPENDS ${PROJECT_NAME}.txt)
# starts in boot ROM, need to set breakpoint before continue
# use "mon reset halt" instead of "starti" to halt at first instruction
