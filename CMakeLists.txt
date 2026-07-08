cmake_minimum_required(VERSION 3.24)
project(microtcp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_executable(microtcp
    src/main.cc
    src/tun/TunDevice.cc
)
target_include_directories(microtcp PRIVATE src)
target_compile_options(microtcp PRIVATE
    -Wall -Wextra -Wpedantic -Wshadow -Wconversion
)
