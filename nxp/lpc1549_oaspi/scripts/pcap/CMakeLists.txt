cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_CXX_STANDARD 23)

if(NOT ${APPLE})
    message(FATAL_ERROR "only tested on macOS")
endif()

project(main)
add_compile_options(-Wall -Wextra -Werror)

find_library(CF CoreFoundation REQUIRED)
find_library(SCF SystemConfiguration REQUIRED)

add_executable(${PROJECT_NAME}
    main.cpp
    eth.cpp
)
target_link_libraries(${PROJECT_NAME} pcap ${CF} ${SCF})
