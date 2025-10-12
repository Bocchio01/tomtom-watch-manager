cmake_minimum_required(VERSION 3.16)

project(cli VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optional flag for local builds
option(LOCAL_BUILD "Build directly in ./build directory" ON)

if(LOCAL_BUILD)
    # Check if we're already inside a build directory
    if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
        message(FATAL_ERROR "In-source builds are not allowed. Please build in ./build/")
    endif()
endif()

# Try to find installed core first
find_package(TomTomWatchCore QUIET)

if(NOT TomTomWatchCore_FOUND)
    # If not found, try to add as subdirectory
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../core/CMakeLists.txt")
        message(STATUS "Using core from parent directory")
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../core core_build)
    else()
        message(FATAL_ERROR "TomTomWatchCore not found. Please build and install core first.")
    endif()
endif()

# Create the CLI executable
add_executable(cli
    app/main.cpp
    src/cli_app.cpp
    src/commands.cpp
)

# Link against the core library
target_link_libraries(cli
    PRIVATE
    TomTomWatch::core
)

target_include_directories(cli
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(cli PROPERTIES
    OUTPUT_NAME "tomtom"
)