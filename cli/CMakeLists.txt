cmake_minimum_required(VERSION 3.16)

project(cli VERSION 0.1 LANGUAGES CXX)

# Optional flag for local builds
option(LOCAL_BUILD "Build directly in ./build directory" ON)

if(LOCAL_BUILD)
    # Check if we're already inside a build directory
    if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
        message(FATAL_ERROR "In-source builds are not allowed. Please build in ./build/")
    endif()

    # Suggest local build path
    message(STATUS "Recommended: build using 'cmake -S . -B build'")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define the include directory for headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add an executable (for now, just a test "hello world")
add_executable(cli
    src/main.cpp
)

# Optionally, if you plan to make it a library later:
# add_library(cli STATIC src/main.cpp)
# target_include_directories(cli PUBLIC ${CMAKE_SOURCE_DIR}/include)
