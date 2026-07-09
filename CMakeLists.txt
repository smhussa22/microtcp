cmake_minimum_required(VERSION 3.24)
project(microtcp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# core library shared by main exe and tests
add_library(microtcp_lib
    src/tun/TunDevice.cc
)
target_include_directories(microtcp_lib PUBLIC src)
target_compile_options(microtcp_lib PRIVATE
    -Wall -Wextra -Wpedantic -Wshadow -Wconversion
)

# main executable
add_executable(microtcp src/main.cc)
target_link_libraries(microtcp PRIVATE microtcp_lib)

# testing
include(FetchContent)
FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_subdirectory(tests)
