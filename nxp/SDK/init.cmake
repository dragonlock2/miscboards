cmake_minimum_required(VERSION 3.20.0)

# compiler
set(CMAKE_SYSTEM_NAME  Generic)
set(CMAKE_C_COMPILER   arm-none-eabi-gcc) # tested on 13.3.rel1
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_OBJCOPY      arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP      arm-none-eabi-objdump)

set(CMAKE_C_COMPILER_WORKS   1)
set(CMAKE_CXX_COMPILER_WORKS 1)

# default options
add_compile_options(
    $<$<COMPILE_LANGUAGE:C>:-std=c23>
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++23>
    -Wall
    -Wextra
    -Werror
    -O3 # use -Og for correct layout for pyOCD GDB
    -g3
    -ffunction-sections
    -fdata-sections
    -ffat-lto-objects
    -flto

    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
)
add_link_options(
    -Wl,--print-memory-usage
    -Wl,--gc-sections
    -O3
    -g3
    -flto=auto
    -nostartfiles
    --specs=nano.specs # comment if using exceptions, need to increase stack size
    --specs=nosys.specs
)

# add libraries
include(${CMAKE_CURRENT_LIST_DIR}/${MCU}/lib.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/freertos/lib.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/freertos_tcp/lib.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/tinyusb/lib.cmake)

# SDK functions
function(sdk_finish)
    # helpers
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".elf")
    add_custom_target(hex ALL ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex DEPENDS ${PROJECT_NAME})
    add_custom_target(disasm ALL ${CMAKE_OBJDUMP} -d -j .text -j .data -j .bss ${PROJECT_NAME}.elf > ${PROJECT_NAME}.txt DEPENDS ${PROJECT_NAME})
    set_property(TARGET hex APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${PROJECT_NAME}.hex)
    set_property(TARGET disasm APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${PROJECT_NAME}.txt)

    sdk_mcu_finish()
endfunction()
