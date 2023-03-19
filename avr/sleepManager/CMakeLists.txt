cmake_minimum_required(VERSION 3.20.0)

# cross-compiling
set(CMAKE_SYSTEM_NAME  Generic)
set(CMAKE_C_COMPILER   avr-gcc)
set(CMAKE_CXX_COMPILER avr-g++)
set(CMAKE_ASM_COMPILER avr-gcc)

# project-specific settings
project(sleepManager)

set(MCU   attiny13a)
set(PROG  snap_isp)
set(F_CPU 1200000L) # factory default 9.6MHz w/ CKDIV8 set

include_directories(
    src/
)

add_compile_definitions(
    F_CPU=${F_CPU}
)

add_compile_options(
    -mmcu=${MCU}
    -std=gnu18
    -Wall
    -Werror
    -O2
    -g
    -funsigned-char
    -funsigned-bitfields
    -fpack-struct
    -fshort-enums
    -ffunction-sections
    -fdata-sections
    -fno-split-wide-types
    -fno-tree-scev-cprop
    -flto
)

add_link_options(
    -mmcu=${MCU}
    -Wl,-script=${CMAKE_CURRENT_SOURCE_DIR}/avr-binutils/ldscripts/avr25.xn
    -Wl,--print-memory-usage
    -Wl,--gc-sections
    -O2
    -g
    -flto
)

file(GLOB SRC_FILES "src/*.c")
add_executable(${PROJECT_NAME}.elf ${SRC_FILES})
add_custom_target(hex ALL avr-objcopy -j .text -j .data -j .rodata -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex DEPENDS ${PROJECT_NAME}.elf)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_NAME}.hex")

add_custom_target(flash avrdude -c ${PROG} -p ${MCU} -U flash:w:${PROJECT_NAME}.hex DEPENDS hex)
