cmake_minimum_required(VERSION 3.20.0)
project(keyboard_sim)

set(CMAKE_CXX_STANDARD 20)

# brew install mbedtls asio
find_package(MbedTLS REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(asio REQUIRED IMPORTED_TARGET asio)

add_library(common common.cpp)
target_link_libraries(common PkgConfig::asio MbedTLS::mbedtls)

add_executable(host host.cpp)
target_link_libraries(host common)

add_executable(device device.cpp)
target_link_libraries(device common)
