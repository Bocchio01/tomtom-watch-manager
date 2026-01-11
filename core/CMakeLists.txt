# =========================================================
# Library definition
# =========================================================

add_library(tomtom_core STATIC
    src/watch.cpp
    src/transport/connection.cpp
    src/transport/connection_factory.cpp
    src/transport/usb/usb_connection.cpp

    # src/transport/ble/ble_connection.cpp
    src/commands/kernel_operations.cpp
    src/commands/file_operations.cpp
)

add_library(TomTomWatch::core ALIAS tomtom_core)

# =========================================================
# Include directories
# =========================================================
target_include_directories(tomtom_core
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# =========================================================
# Compiler setup
# =========================================================
target_compile_features(tomtom_core PUBLIC cxx_std_17)
set_target_properties(tomtom_core PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_EXTENSIONS OFF
)

# =========================================================
# Platform-specific sources and libraries
# =========================================================
if(UNIX AND NOT APPLE)
    target_sources(tomtom_core PRIVATE
        src/transport/usb/usb_connection_unix.cpp
    )
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBUSB REQUIRED libusb-1.0)
    find_package(CURL REQUIRED)
    target_link_libraries(tomtom_core PUBLIC ${LIBUSB_LIBRARIES} CURL::libcurl)
    target_include_directories(tomtom_core PUBLIC ${LIBUSB_INCLUDE_DIRS})

elseif(WIN32)
    target_sources(tomtom_core PRIVATE
        src/transport/usb/usb_connection_windows.cpp
    )
    target_compile_definitions(tomtom_core PRIVATE
        UNICODE
        _UNICODE
    )
    target_link_libraries(tomtom_core PUBLIC setupapi winusb hid winhttp)

elseif(APPLE)
    target_sources(tomtom_core PRIVATE
        src/transport/usb/usb_connection_macos.cpp
    )
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation REQUIRED)
    find_package(CURL REQUIRED)
    target_link_libraries(tomtom_core PUBLIC ${IOKIT_LIBRARY} ${COREFOUNDATION_LIBRARY} CURL::libcurl)
endif()
