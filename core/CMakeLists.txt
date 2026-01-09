# ---------------------------------------------------------
# Library definition
# ---------------------------------------------------------

add_library(tomtom_core STATIC
    src/watch.cpp
    src/manager.cpp
    src/transport/connection.cpp
    src/transport/connection_factory.cpp
    src/transport/usb/usb_connection.cpp
    src/protocol/services/file_service.cpp
    src/protocol/services/watch_control_service.cpp
    src/protocol/services/watch_info_service.cpp

    # File domain services
    src/files/activity/activity_parser.cpp
    src/files/activity/activity_service.cpp
    src/files/preferences/preferences_parser.cpp
    src/files/preferences/preferences_serializer.cpp
    src/files/preferences/preferences_service.cpp
    src/files/tracking/tracking_service.cpp
    src/files/routes/route_service.cpp
    src/files/manifest/manifest_parser.cpp
    src/files/manifest/manifest_service.cpp
    src/files/workouts/workouts_parser.cpp
    src/files/workouts/workouts_service.cpp
)

add_library(TomTomWatch::core ALIAS tomtom_core)

# ---------------------------------------------------------
# Include directories
# ---------------------------------------------------------
target_include_directories(tomtom_core
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>

    $<INSTALL_INTERFACE:include>
)

# ---------------------------------------------------------
# Compiler setup
# ---------------------------------------------------------
target_compile_features(tomtom_core PUBLIC cxx_std_17)
set_target_properties(tomtom_core PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_EXTENSIONS OFF
)

# ---------------------------------------------------------
# Optional logging support (spdlog is fetched in root)
# ---------------------------------------------------------
target_link_libraries(tomtom_core
    PRIVATE spdlog::spdlog
)
target_compile_definitions(tomtom_core PRIVATE
    ENABLE_LOGGING
    SPDLOG_HEADER_ONLY
)

# Note: Protobuf integration will be completed when proto files are generated
# See core/proto/README.md for instructions on generating C++ code from .proto files

# Platform-specific sources
if(UNIX AND NOT APPLE)
    target_sources(tomtom_core PRIVATE
        src/transport/usb/usb_connection_unix.cpp
    )
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBUSB REQUIRED libusb-1.0)
    target_link_libraries(tomtom_core PUBLIC ${LIBUSB_LIBRARIES})
    target_include_directories(tomtom_core PUBLIC ${LIBUSB_INCLUDE_DIRS})
elseif(WIN32)
    target_sources(tomtom_core PRIVATE
        src/transport/usb/usb_connection_windows.cpp
    )
    target_compile_definitions(tomtom_core PRIVATE
        UNICODE
        _UNICODE
    )
    target_link_libraries(tomtom_core PUBLIC setupapi winusb hid)

elseif(APPLE)
    target_sources(tomtom_core PRIVATE
        src/transport/usb/usb_connection_macos.cpp
    )
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation REQUIRED)
    target_link_libraries(tomtom_core PUBLIC ${IOKIT_LIBRARY} ${COREFOUNDATION_LIBRARY})
endif()