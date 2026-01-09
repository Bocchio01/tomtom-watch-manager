cmake_minimum_required(VERSION 3.15)
project(TomTomWatchManager VERSION 1.0 LANGUAGES CXX)

# =========================================================
# Build options
# =========================================================
option(BUILD_CORE "Build core library" ON)
option(BUILD_CLI "Build CLI client" OFF)
option(BUILD_GUI "Build GUI client" OFF)

option(BUILD_TESTS "Build test executables" ON)
option(BUILD_EXAMPLES "Build example executables" OFF)

option(GENERATE_DOCS "Generate documentation with Doxygen" OFF)

# =========================================================
# Global settings
# =========================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable Link-Time Optimization only for Release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# =========================================================
# Prevent in-source builds
# =========================================================
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed. Please create a build directory.")
endif()

# =========================================================
# Output directories
# =========================================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# =========================================================
# Installation directories
# =========================================================
include(GNUInstallDirs)

# =========================================================
# FetchContent setup (centralized)
# =========================================================
include(FetchContent)

# Speed up FetchContent by using shallow clones
set(FETCHCONTENT_QUIET OFF)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON) # Don't update every time

# Declare all dependencies upfront
message(STATUS "Fetching spdlog...")
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.16.0
    GIT_SHALLOW TRUE
)

# Configure spdlog to be header-only to avoid Windows linking issues
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SPDLOG_INSTALL OFF CACHE BOOL "" FORCE)

# Make all declared dependencies available at once
set(_fetch_deps "")
list(APPEND _fetch_deps spdlog)

if(_fetch_deps)
    message(STATUS "Making available FetchContent dependencies: ${_fetch_deps}")
    FetchContent_MakeAvailable(${_fetch_deps})
endif()

# =========================================================
# Add subdirectories
# =========================================================
if(BUILD_CORE)
    message(STATUS "Adding core library...")
    add_subdirectory(core)
endif()

if(BUILD_CLI)
    message(STATUS "Adding CLI client...")
    add_subdirectory(ui/cli)
endif()

if(BUILD_GUI)
    message(STATUS "Adding GUI client...")
    add_subdirectory(ui/gui)
endif()

# =========================================================
# Testing setup
# =========================================================
if(BUILD_TESTS)
    enable_testing()
    message(STATUS "Testing enabled - use 'ctest' to run tests")
    add_subdirectory(tests)
endif()

# =========================================================
# Examples setup
# =========================================================
if(BUILD_EXAMPLES)
    message(STATUS "Adding examples...")
    add_subdirectory(examples)
endif()

# =========================================================
# Documentation setup
# =========================================================
find_package(Doxygen QUIET)

if(DOXYGEN_FOUND AND GENERATE_DOCS)
    FetchContent_Declare(
        doxygen-awesome-css
        URL https://github.com/jothepro/doxygen-awesome-css/archive/refs/heads/main.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(doxygen-awesome-css)
    FetchContent_GetProperties(doxygen-awesome-css SOURCE_DIR AWESOME_CSS_DIR)

    set(DOXYGEN_INPUT ${CMAKE_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUTPUT ${CMAKE_BINARY_DIR}/docs/Doxyfile)
    configure_file(${DOXYGEN_INPUT} ${DOXYGEN_OUTPUT} @ONLY)

    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUTPUT}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
elseif(GENERATE_DOCS AND NOT DOXYGEN_FOUND)
    message(WARNING "Doxygen not found. Install it to enable documentation generation.")
endif()

# =========================================================
# Print configuration summary
# =========================================================
message(STATUS "")
message(STATUS "========================================")
message(STATUS "   TomTom Watch Manager Configuration   ")
message(STATUS "========================================")
message(STATUS " Version: ${PROJECT_VERSION}")
message(STATUS " Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS " C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "")
message(STATUS "Components:")
message(STATUS " - Core library:      ${BUILD_CORE}")
message(STATUS " - CLI client:        ${BUILD_CLI}")
message(STATUS " - GUI client:        ${BUILD_GUI}")
message(STATUS "")
message(STATUS "Options:")
message(STATUS " - Build tests:       ${BUILD_TESTS}")
message(STATUS " - Build examples:    ${BUILD_EXAMPLES}")
message(STATUS "")
message(STATUS "Output:")
message(STATUS " - Executables:       ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS " - Libraries:         ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
message(STATUS "")
message(STATUS "Installation dirs:")
message(STATUS " - Binaries:          ${CMAKE_INSTALL_BINDIR}")
message(STATUS " - Libraries:         ${CMAKE_INSTALL_LIBDIR}")
message(STATUS " - Includes:          ${CMAKE_INSTALL_INCLUDEDIR}")
message(STATUS "========================================")
message(STATUS "")