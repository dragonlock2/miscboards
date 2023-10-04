cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_C_COMPILER       ${CMAKE_CURRENT_LIST_DIR}/../toolchain/gcc/bin/riscv-none-embed-gcc)
set(CMAKE_CXX_COMPILER     ${CMAKE_CURRENT_LIST_DIR}/../toolchain/gcc/bin/riscv-none-embed-g++)
set(CMAKE_ASM_COMPILER     ${CMAKE_CURRENT_LIST_DIR}/../toolchain/gcc/bin/riscv-none-embed-gcc)
set(CMAKE_OBJCOPY_COMPILER ${CMAKE_CURRENT_LIST_DIR}/../toolchain/gcc/bin/riscv-none-embed-objcopy)
set(CMAKE_OBJDUMP_COMPILER ${CMAKE_CURRENT_LIST_DIR}/../toolchain/gcc/bin/riscv-none-embed-objdump)

set(OPENOCD_BIN ${CMAKE_CURRENT_LIST_DIR}/../toolchain/openocd/bin/openocd)
set(OPENOCD_CFG ${CMAKE_CURRENT_LIST_DIR}/../toolchain/openocd/bin/wch-riscv.cfg)

set(CMAKE_SYSTEM_NAME        Generic)
set(CMAKE_C_COMPILER_WORKS   1)
set(CMAKE_CXX_COMPILER_WORKS 1)

set(WCH_SDK_DIR "${CMAKE_CURRENT_LIST_DIR}/ch32v003/EVT/EXAM/SRC")

# default build options
add_compile_options(
    -march=rv32ec
    -mabi=ilp32e
    -mcmodel=medany
)

add_link_options(
    -march=rv32ec
    -mabi=ilp32e
    -Wl,-script=${WCH_SDK_DIR}/Ld/Link.ld
    -mcmodel=medany
    -nostartfiles
    --specs=nano.specs
    --specs=nosys.specs
)

# generate hex
if (NOT TARGET hex)
    add_custom_target(hex ALL ${CMAKE_OBJCOPY_COMPILER} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex DEPENDS ${PROJECT_NAME}.elf)
    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_NAME}.hex")
endif()

# generate objdump
if (NOT TARGET dump)
    add_custom_target(dump ALL ${CMAKE_OBJDUMP_COMPILER} -d ${PROJECT_NAME}.elf > ${PROJECT_NAME}.txt DEPENDS ${PROJECT_NAME}.elf)
    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_NAME}.txt")
endif()

# debug helpers
if (NOT TARGET flash)
    add_custom_target(flash  ${OPENOCD_BIN} -f ${OPENOCD_CFG} -c init -c halt -c "flash write_image ${PROJECT_NAME}.elf" -c reset -c exit DEPENDS ${PROJECT_NAME}.elf)
    add_custom_target(server ${OPENOCD_BIN} -f ${OPENOCD_CFG} -c init -c halt DEPENDS ${PROJECT_NAME}.elf)
    add_custom_target(gdb    gdb ${PROJECT_NAME}.elf -ex "tar ext localhost:3333" -ex "load" DEPENDS ${PROJECT_NAME}.elf)
endif()
