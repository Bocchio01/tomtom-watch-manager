cmake_minimum_required(VERSION 3.16)

project(core VERSION 0.1 LANGUAGES CXX)

# Optional flag for local builds
option(LOCAL_BUILD "Build directly in ./build directory" ON)

if(LOCAL_BUILD)
    if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
        message(FATAL_ERROR "In-source builds are not allowed. Please build in ./build/")
    endif()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define the include directory for headers
include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(core STATIC
    src/watch.cpp
    src/manager.cpp
    src/connection/usb_connection.cpp
)

# Platform-specific sources
if(UNIX AND NOT APPLE)
    target_sources(core PRIVATE
        src/connection/platforms/linux_usb.cpp
    )
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBUSB REQUIRED libusb-1.0)
    target_link_libraries(core PUBLIC ${LIBUSB_LIBRARIES})
    target_include_directories(core PUBLIC ${LIBUSB_INCLUDE_DIRS})
elseif(WIN32)
    target_sources(core PRIVATE
        src/connection/platforms/windows_usb.cpp
    )
    target_link_libraries(core PUBLIC setupapi winusb)
elseif(APPLE)
    target_sources(core PRIVATE
        src/connection/platforms/macos_usb.cpp
    )
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation REQUIRED)
    target_link_libraries(core PUBLIC ${IOKIT_LIBRARY} ${COREFOUNDATION_LIBRARY})
endif()

target_include_directories(core
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# =========================================================
# Testing setup
# =========================================================
option(BUILD_TESTS "Build test executables" ON)

if(BUILD_TESTS)
    enable_testing()

    file(GLOB TEST_SOURCES test/*.cpp)

    foreach(test_src ${TEST_SOURCES})
        get_filename_component(test_name ${test_src} NAME_WE)
        add_executable(${test_name} ${test_src})
        target_link_libraries(${test_name} PRIVATE core)
        add_test(NAME ${test_name} COMMAND ${test_name})
    endforeach()
endif()