cmake_minimum_required(VERSION 3.20.0)

# SDK copied from LPCXpresso LPC1769 board v2.10, modified to fix some warnings
# https://www.nxp.com/design/design-center/software/software-library/lpcopen-software-development-platform-lpc17xx:LPCOPEN-SOFTWARE-FOR-LPC17XX

# default options
add_compile_definitions(
    CORE_M3
)
add_compile_options(
    -mcpu=cortex-m3
)
add_link_options(
    -mcpu=cortex-m3
    -Wl,-script=${CMAKE_CURRENT_LIST_DIR}/linker.ld
)

# bootstrap lib
file(GLOB SDK_BOOTSTRAP_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/startup.c
    ${CMAKE_CURRENT_LIST_DIR}/src/*.c
)
add_library(bootstrap ${SDK_BOOTSTRAP_SRCS})
target_include_directories(bootstrap SYSTEM PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/inc
)
set_target_properties(bootstrap PROPERTIES EXCLUDE_FROM_ALL TRUE)

# SDK functions
function(sdk_mcu_finish)
    # pyocd pack install lpc1768
    add_custom_target(flash  pyocd load ${PROJECT_NAME}.elf -t lpc1768 DEPENDS ${PROJECT_NAME}.elf)
    add_custom_target(server pyocd gdbserver --persist -f 4MHz -t lpc1768 DEPENDS ${PROJECT_NAME}.elf)
    add_custom_target(gdb    arm-none-eabi-gdb ${PROJECT_NAME}.elf -ex "tar ext localhost:3333" -ex "load"
        -ex "mon reset halt" -ex "set mem inaccessible-by-default off" DEPENDS ${PROJECT_NAME}.txt)
    # starts in boot ROM, need to set breakpoint before continue
    # use "mon reset halt" instead of "starti" to halt at first instruction
endfunction()
