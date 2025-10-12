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

include(FetchContent)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.3 # (or the latest stable tag)
)

FetchContent_MakeAvailable(spdlog)

target_link_libraries(core PRIVATE spdlog::spdlog)

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
    target_compile_definitions(core PRIVATE
        UNICODE
        _UNICODE
    )
    target_link_libraries(core PUBLIC setupapi winusb hid)

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
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(core PROPERTIES
    POSITION_INDEPENDENT_CODE ON
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
        target_link_libraries(${test_name} PRIVATE core spdlog::spdlog)
        add_test(NAME ${test_name} COMMAND ${test_name})
    endforeach()
endif()

# =========================================================
# Doxygen documentation
# =========================================================
find_package(Doxygen)
option(GENERATED_DOCS "Generate documentation with Doxygen" ON)

if(DOXYGEN_FOUND AND GENERATED_DOCS)
    add_custom_target(doxygen
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)

    message(STATUS "Doxygen build enabled. Run `make doxygen` (or `cmake --build . --target doxygen`).")
else()
    message(WARNING "Doxygen not found. Documentation target will not be available.")
endif()
