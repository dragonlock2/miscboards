set(PICO_SDK_PATH ${CMAKE_CURRENT_LIST_DIR}/pico-sdk)
include(${PICO_SDK_PATH}/pico_sdk_init.cmake)

if(PICO_PLATFORM STREQUAL "rp2350-riscv")
    set(PICO_TOOLCHAIN_PATH ${CMAKE_CURRENT_LIST_DIR}/../SDK/riscv-toolchain)
endif()

set(CMAKE_C_STANDARD   23)
set(CMAKE_CXX_STANDARD 23)

function(pico_finalize)
    # stdio usb helpers
    get_target_property(PICO_TARGET_STDIO_USB ${PROJECT_NAME} PICO_TARGET_STDIO_USB)
    if(PICO_TARGET_STDIO_USB EQUAL 1 AND TARGET freertos)
        pico_enable_stdio_usb(freertos 1)
    endif()

    # pico_w helpers
    if(PICO_CYW43_SUPPORTED AND TARGET freertos)
        target_link_libraries(${PROJECT_NAME}
            pico_cyw43_arch_sys_freertos
        )
    endif()

    # boilerplate helpers
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
    pico_add_extra_outputs(${PROJECT_NAME})
    add_custom_target(flash picotool load -f ${PROJECT_NAME}.uf2 DEPENDS ${PROJECT_NAME})
endfunction()

set_source_files_properties( # disable warnings for some files
    ${PICO_SDK_PATH}/src/rp2_common/pico_async_context/async_context_freertos.c
    ${PICO_SDK_PATH}/src/rp2_common/pico_cyw43_driver/cyw43_driver.c
    PROPERTIES APPEND_STRING PROPERTY COMPILE_FLAGS " -w"
)
