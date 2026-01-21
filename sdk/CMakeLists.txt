# ============================================================================
# Library Target
# ============================================================================
add_library(tomtom_sdk
    src/manager.cpp

    # Parsers
    src/parsers/activity_parser.cpp
    src/parsers/preferences_parser.cpp

    # Serializers
    src/serializers/preferences_serializer.cpp

    # Services
    src/services/activity_service.cpp
    src/services/preferences_service.cpp
    src/services/gps_service.cpp
    src/services/device_service.cpp

    # Converters
    src/converters/gpx_converter.cpp
    src/converters/kml_converter.cpp
    src/converters/json_converter.cpp

    # Store
    src/store/local_store.cpp

    # Utils
    src/utils/http_client.cpp
)

add_library(TomTomWatch::sdk ALIAS tomtom_sdk)

# ============================================================================
# Include Directories
# ============================================================================
target_include_directories(tomtom_sdk
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# ============================================================================
# Dependencies
# ============================================================================

# # Use header-only spdlog
# target_compile_definitions(tomtom_sdk PRIVATE SPDLOG_HEADER_ONLY)

# Link against core library
target_link_libraries(tomtom_sdk
    PUBLIC
    TomTomWatch::core
    PRIVATE
    spdlog::spdlog_header_only
    nlohmann_json::nlohmann_json
)

# Windows: Link WinHTTP for HTTP downloads
if(WIN32)
    target_link_libraries(tomtom_sdk PRIVATE winhttp)
endif()

# Unix: Link libcurl for HTTP downloads
if(UNIX)
    find_package(CURL REQUIRED)
    target_link_libraries(tomtom_sdk PRIVATE CURL::libcurl)
endif()

# =========================================================
# Compiler setup
# =========================================================
target_compile_features(tomtom_sdk PUBLIC cxx_std_17)
set_target_properties(tomtom_sdk PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_EXTENSIONS OFF
)