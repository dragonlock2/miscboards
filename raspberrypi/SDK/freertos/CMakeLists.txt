cmake_minimum_required(VERSION 3.20.0)

# TODO update once rp2350 support in official release
set(FREERTOS_KERNEL_PATH           ../../../shared/FreeRTOS-Kernel)
set(FREERTOS_KERNEL_COMMUNITY_PATH ../../../shared/FreeRTOS-Kernel-Community-Supported-Ports)

if(PICO_PLATFORM STREQUAL "rp2040")
    set(FREERTOS_KERNEL_PORT_SRC ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/port.c)
    set(FREERTOS_KERNEL_PORT_INC ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/include)
elseif(PICO_PLATFORM STREQUAL "rp2350-arm-s" OR PICO_PLATFORM STREQUAL "rp2350")
    set(FREERTOS_KERNEL_PORT_SRC ${FREERTOS_KERNEL_COMMUNITY_PATH}/GCC/RP2350_ARM_NTZ/non_secure/*.c)
    set(FREERTOS_KERNEL_PORT_INC ${FREERTOS_KERNEL_COMMUNITY_PATH}/GCC/RP2350_ARM_NTZ/non_secure)
elseif(PICO_PLATFORM STREQUAL "rp2350-riscv")
    set(FREERTOS_KERNEL_PORT_SRC
        ${FREERTOS_KERNEL_COMMUNITY_PATH}/GCC/RP2350_RISC-V/*.c
        ${FREERTOS_KERNEL_COMMUNITY_PATH}/GCC/RP2350_RISC-V/*.S
    )
    set(FREERTOS_KERNEL_PORT_INC ${FREERTOS_KERNEL_COMMUNITY_PATH}/GCC/RP2350_RISC-V/include)
else()
    message(FATAL_ERROR "unknown platform")
endif()

file(GLOB FREERTOS_SRCS
    main.c
    ${FREERTOS_KERNEL_PATH}/*.c
    ${FREERTOS_KERNEL_PATH}/portable/MemMang/heap_4.c
    ${FREERTOS_KERNEL_PORT_SRC}
)
add_library(freertos ${FREERTOS_SRCS})
    
target_include_directories(freertos SYSTEM PUBLIC
    .
    ${FREERTOS_KERNEL_PATH}/include
    ${FREERTOS_KERNEL_PORT_INC}
)

target_compile_definitions(freertos PUBLIC
    LIB_FREERTOS_KERNEL=1
    FREE_RTOS_KERNEL_SMP=1
    PICO_CONFIG_RTOS_ADAPTER_HEADER=${FREERTOS_KERNEL_PORT_INC}/freertos_sdk_config.h

    CYW43_TASK_STACK_SIZE=1024
    NO_SYS=0
    CYW43_LWIP=0
)

target_link_libraries(freertos PUBLIC
    pico_base_headers
    pico_stdlib
    hardware_clocks
    hardware_exception
    pico_multicore
    $<$<STREQUAL:${PICO_PLATFORM},rp2350-riscv>:hardware_riscv_platform_timer>
    $<$<BOOL:${PICO_CYW43_SUPPORTED}>:pico_cyw43_arch_sys_freertos>
)
