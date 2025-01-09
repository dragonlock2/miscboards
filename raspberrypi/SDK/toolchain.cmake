include(${CMAKE_CURRENT_LIST_DIR}/pico-sdk/pico_sdk_init.cmake)

# opinionated set of options :)
set(CMAKE_C_STANDARD   23)
set(CMAKE_CXX_STANDARD 23)

add_compile_options(
    -Wall
    -Wextra
    -Werror
)

add_link_options(
    -Wl,--print-memory-usage
)

if(PICO_PLATFORM STREQUAL "rp2350-riscv")
    set(PICO_TOOLCHAIN_PATH ${CMAKE_CURRENT_LIST_DIR}/../SDK/riscv-toolchain)
endif()

function(pico_finalize)
    # need to call at end
    get_target_property(PICO_TARGET_STDIO_USB ${PROJECT_NAME} PICO_TARGET_STDIO_USB)
    if(PICO_TARGET_STDIO_USB EQUAL 1 AND TARGET freertos)
        pico_enable_stdio_usb(freertos 1)
    endif()
    target_link_libraries(${PROJECT_NAME}
        pico_stdlib
    )
    pico_add_extra_outputs(${PROJECT_NAME})
    add_custom_target(flash picotool load -f ${PROJECT_NAME}.uf2 DEPENDS ${PROJECT_NAME})
endfunction()
