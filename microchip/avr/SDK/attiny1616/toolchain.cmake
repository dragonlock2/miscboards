cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_SYSTEM_NAME  Generic)
set(CMAKE_C_COMPILER   avr-gcc) # tested on 14.1.0
set(CMAKE_CXX_COMPILER avr-g++)
set(CMAKE_ASM_COMPILER avr-gcc)
set(CMAKE_OBJCOPY      avr-objcopy)
set(CMAKE_OBJDUMP      avr-objdump)

add_compile_options(
    -mmcu=attiny1616
    -B${CMAKE_CURRENT_LIST_DIR}/dfp
    -fshort-enums # minimum width for enums
)

add_link_options(
    -mmcu=attiny1616
    -B${CMAKE_CURRENT_LIST_DIR}/dfp
)

# generate hex
if (NOT TARGET ${PROJECT_NAME}.hex)
    add_custom_target(${PROJECT_NAME}.hex ALL ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex DEPENDS ${PROJECT_NAME}.elf)
    set_property(TARGET ${PROJECT_NAME}.hex APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${PROJECT_NAME}.hex)
endif()
    
# generate objdump
if (NOT TARGET ${PROJECT_NAME}.txt)
    add_custom_target(${PROJECT_NAME}.txt ALL ${CMAKE_OBJDUMP} -D ${PROJECT_NAME}.elf > ${PROJECT_NAME}.txt DEPENDS ${PROJECT_NAME}.elf)
    set_property(TARGET ${PROJECT_NAME}.txt APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${PROJECT_NAME}.txt)
endif()

# debug helpers
if (NOT TARGET flash)
    add_custom_target(flash avrdude -c snap_updi -p attiny1616 -U flash:w:${PROJECT_NAME}.hex DEPENDS ${PROJECT_NAME}.hex)
endif()
