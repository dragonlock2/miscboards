cmake_minimum_required(VERSION 3.20.0)

# auto find the correct version
file(GLOB TOOLCHAIN_DIRS LIST_DIRECTORIES true "$ENV{HOME}/Library/xPacks/@xpack-dev-tools/riscv-none-elf-gcc/*")
list(GET TOOLCHAIN_DIRS 0 TOOLCHAIN_DIR)
set(GCC_DIR ${TOOLCHAIN_DIR}/.content/bin)

set(CMAKE_SYSTEM_NAME  Generic)
set(CMAKE_C_COMPILER   ${GCC_DIR}/riscv-none-elf-gcc)
set(CMAKE_CXX_COMPILER ${GCC_DIR}/riscv-none-elf-g++)
set(CMAKE_ASM_COMPILER ${GCC_DIR}/riscv-none-elf-gcc)
set(CMAKE_AR           ${GCC_DIR}/riscv-none-elf-ar)
set(CMAKE_SIZE         ${GCC_DIR}/riscv-none-elf-size)
set(CMAKE_OBJCOPY      ${GCC_DIR}/riscv-none-elf-objcopy)
set(CMAKE_OBJDUMP      ${GCC_DIR}/riscv-none-elf-objdump)

set(OPENOCD_BIN ${CMAKE_CURRENT_LIST_DIR}/../riscv-openocd/src/openocd)
set(OPENOCD_CFG ${CMAKE_CURRENT_LIST_DIR}/../wch-riscv.cfg)

set(WCH_SDK_DIR   ${CMAKE_CURRENT_LIST_DIR}/ch32v003/EVT/EXAM/SRC)
set(LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/linker.ld)

# default build options
add_compile_options(
    -march=rv32ec_zicsr
    -mabi=ilp32e
    -mcmodel=medany
    $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
)

add_link_options(
    -march=rv32ec_zicsr
    -mabi=ilp32e
    -mcmodel=medany
    -Wl,-script=${LINKER_SCRIPT}
    -Wl,--no-warn-rwx-segments
    -nostartfiles
)

# generate hex
if (NOT TARGET ${PROJECT_NAME}.hex)
    add_custom_target(${PROJECT_NAME}.hex ALL ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex DEPENDS ${PROJECT_NAME}.elf)
    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_NAME}.hex")
endif()

# generate objdump
if (NOT TARGET ${PROJECT_NAME}.txt)
    add_custom_target(${PROJECT_NAME}.txt ALL ${CMAKE_OBJDUMP} -d -j .text -j .data ${PROJECT_NAME}.elf > ${PROJECT_NAME}.txt DEPENDS ${PROJECT_NAME}.elf)
    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_NAME}.txt")
endif()

# debug helpers
if (NOT TARGET flash)
    add_custom_target(flash  ${OPENOCD_BIN} -f ${OPENOCD_CFG} -c init -c halt -c "flash write_image ${PROJECT_NAME}.elf" -c reset -c exit DEPENDS ${PROJECT_NAME}.elf)
    add_custom_target(server ${OPENOCD_BIN} -f ${OPENOCD_CFG} -c init -c halt DEPENDS ${PROJECT_NAME}.elf)
    add_custom_target(gdb    gdb ${PROJECT_NAME}.elf -ex "tar ext localhost:3333" -ex "load" DEPENDS ${PROJECT_NAME}.elf)
endif()
