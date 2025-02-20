set(PICO_SDK_PATH    ${CMAKE_CURRENT_LIST_DIR}/pico-sdk)
set(PICO_EXTRAS_PATH ${CMAKE_CURRENT_LIST_DIR}/pico-extras)
include(${PICO_SDK_PATH}/pico_sdk_init.cmake)
include(${PICO_EXTRAS_PATH}/external/pico_extras_import.cmake)

# OpenOCD settings
set(OPENOCD_PATH ${CMAKE_CURRENT_LIST_DIR}/openocd)
set(OPENOCD_BIN  ${OPENOCD_PATH}/src/openocd -s ${OPENOCD_PATH}/tcl -f interface/cmsis-dap.cfg)

if(PICO_PLATFORM STREQUAL "rp2040")
    set(OPENOCD_TARGET rp2040.cfg)
elseif(PICO_PLATFORM STREQUAL "rp2350-arm-s" OR PICO_PLATFORM STREQUAL "rp2350")
    set(OPENOCD_TARGET rp2350.cfg)
elseif(PICO_PLATFORM STREQUAL "rp2350-riscv")
    set(PICO_TOOLCHAIN_PATH ${CMAKE_CURRENT_LIST_DIR}/../SDK/riscv-toolchain)
    set(OPENOCD_TARGET rp2350-riscv.cfg)
else()
    message(FATAL_ERROR "unknown platform")
endif()

# default options
set(CMAKE_C_STANDARD   23)
set(CMAKE_CXX_STANDARD 23)

# add libraries
include(${CMAKE_CURRENT_LIST_DIR}/freertos/lib.cmake)

# SDK functions
function(sdk_init)
    pico_sdk_init()
endfunction()

function(sdk_finish)
    # stdio usb helpers
    get_target_property(PICO_TARGET_STDIO_USB ${PROJECT_NAME} PICO_TARGET_STDIO_USB)
    sdk_freertos_enabled(FREERTOS_ENABLED)
    if(PICO_TARGET_STDIO_USB EQUAL 1 AND FREERTOS_ENABLED)
        pico_enable_stdio_usb(freertos 1)
    endif()

    # pico_w helpers
    if(PICO_CYW43_SUPPORTED AND FREERTOS_ENABLED)
        target_link_libraries(${PROJECT_NAME} pico_cyw43_arch_sys_freertos)
    endif()

    # default options
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Werror
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        -Wl,--print-memory-usage
    )
    target_link_libraries(${PROJECT_NAME}
        pico_stdlib
    )

    # helpers
    pico_add_extra_outputs(${PROJECT_NAME})
    add_custom_target(flash picotool load -f ${PROJECT_NAME}.uf2 DEPENDS ${PROJECT_NAME})
    add_custom_target(flash_dbg ${OPENOCD_BIN} -f target/${OPENOCD_TARGET} -c "adapter speed 5000"
        -c "program ${PROJECT_NAME}.elf verify reset exit" DEPENDS ${PROJECT_NAME})
    add_custom_target(server ${OPENOCD_BIN} -f target/${OPENOCD_TARGET} -c "adapter speed 5000" DEPENDS ${PROJECT_NAME})
    add_custom_target(server_rtt ${OPENOCD_BIN} -f target/${OPENOCD_TARGET} -c "adapter speed 5000"
        -c "rtt setup 0x20000000 8192 \"SEGGER RTT\"" -c "init" -c "rtt start" -c "rtt server start 19021 0" DEPENDS ${PROJECT_NAME})
    add_custom_target(gdb gdb ${PROJECT_NAME}.elf -ex "tar ext localhost:3333" -ex "set mem inaccessible-by-default on" DEPEND ${PROJECT_NAME})
endfunction()

# disable warnings for some files
set_source_files_properties(
    ${PICO_SDK_PATH}/src/rp2_common/pico_async_context/async_context_freertos.c
    ${PICO_SDK_PATH}/src/rp2_common/pico_cyw43_driver/cyw43_driver.c
    ${PICO_SDK_PATH}/lib/btstack/src/ble/sm.c
    ${PICO_EXTRAS_PATH}/src/rp2_common/pico_sleep/sleep.c
    PROPERTIES APPEND_STRING PROPERTY COMPILE_FLAGS " -w"
)
