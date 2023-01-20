cmake_minimum_required(VERSION 3.20.0)

# cross-compiling
set(CMAKE_SYSTEM_NAME  Generic)
set(CMAKE_C_COMPILER   avr-gcc)
set(CMAKE_CXX_COMPILER avr-g++)
set(CMAKE_ASM_COMPILER avr-gcc)

# project-specific settings
project(lightsaber)

set(MCU  attiny817)
set(PROG xplainedmini_updi)

include_directories(
    dfp/include
)

add_compile_options(
    -mmcu=${MCU}
    -B${CMAKE_CURRENT_SOURCE_DIR}/dfp/gcc/dev/${MCU}
    -std=gnu18
    -Wall
    -Werror
    -Os
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
    -B${CMAKE_CURRENT_SOURCE_DIR}/dfp/gcc/dev/${MCU}
    -L${CMAKE_CURRENT_SOURCE_DIR}/avr-libc/avrxmega3
    -Wl,--print-memory-usage
    -Wl,--gc-sections
    -Os
    -g
    -flto
)

file(GLOB SRC_FILES "src/*.c")
add_executable(${PROJECT_NAME}.elf ${SRC_FILES})
add_custom_target(hex ALL avr-objcopy -j .text -j .data -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex DEPENDS ${PROJECT_NAME}.elf)

add_custom_target(flash avrdude -c ${PROG} -p ${MCU} -U flash:w:${PROJECT_NAME}.hex DEPENDS hex)
