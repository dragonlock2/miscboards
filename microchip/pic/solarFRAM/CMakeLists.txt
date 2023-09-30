cmake_minimum_required(VERSION 3.20.0)

set(MICROCHIP_MCU PIC12F1571)
set(MICROCHIP_XC8_PATH /usr/local/Caskroom/mplab-xc8/2.41)
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)
set(CMAKE_TOOLCHAIN_FILE ../cmake-microchip/toolchain.cmake)

project(solarFRAM)

file(GLOB SRC_FILES "src/*.c")
add_executable(${PROJECT_NAME} ${SRC_FILES})

set(IPECMD /Applications/microchip/mplabx/v6.05/mplab_platform/mplab_ipe/bin/ipecmd.sh)
add_custom_target(flash ${IPECMD} -P12F1571 -TPSNAP -M -F${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.hex -OL DEPENDS ${PROJECT_NAME})
