cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_SYSTEM_NAME  Generic)
set(CMAKE_C_COMPILER   arm-none-eabi-gcc) # tested on 13.3.rel1
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_OBJCOPY      arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP      arm-none-eabi-objdump)

set(CMAKE_C_COMPILER_WORKS   1)
set(CMAKE_CXX_COMPILER_WORKS 1)

add_compile_definitions(
    CORE_M3
)

add_compile_options(
    -mcpu=cortex-m3
    $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
)

add_link_options(
    -mcpu=cortex-m3
    -nostartfiles
    -Wl,-script=${CMAKE_CURRENT_LIST_DIR}/linker.ld
)

# generate hex
if (NOT TARGET ${PROJECT_NAME}.hex)
    add_custom_target(${PROJECT_NAME}.hex ALL ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex DEPENDS ${PROJECT_NAME}.elf)
    set_property(TARGET ${PROJECT_NAME}.hex APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${PROJECT_NAME}.hex)
endif()

# generate objdump
if (NOT TARGET ${PROJECT_NAME}.txt)
    add_custom_target(${PROJECT_NAME}.txt ALL ${CMAKE_OBJDUMP} -d -j .text -j .data -j .bss ${PROJECT_NAME}.elf > ${PROJECT_NAME}.txt DEPENDS ${PROJECT_NAME}.elf)
    set_property(TARGET ${PROJECT_NAME}.txt APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${PROJECT_NAME}.txt)
endif()

# debug helpers
if (NOT TARGET flash)
    add_custom_target(flash  pyocd load ${PROJECT_NAME}.elf -f 4MHz -t lpc1549jbd48 DEPENDS ${PROJECT_NAME}.elf)
    add_custom_target(server pyocd gdbserver --persist -f 4MHz -t lpc1549jbd48 DEPENDS ${PROJECT_NAME}.elf)
    add_custom_target(gdb    arm-none-eabi-gdb ${PROJECT_NAME}.elf -ex "tar ext localhost:3333" -ex "load"
        -ex "mon reset halt" -ex "set mem inaccessible-by-default off" DEPENDS ${PROJECT_NAME}.txt)
    # starts in boot ROM, need to set breakpoint before continue
    # use "mon reset halt" instead of "starti" to halt at first instruction
endif()
