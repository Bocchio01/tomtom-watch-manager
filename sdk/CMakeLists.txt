# ============================================================================
# Library Target
# ============================================================================
add_library(tomtom_sdk

    # Activity management
    src/activity_converter.cpp
    src/activity_manager.cpp

    # GPS QuickFix management
    src/quickfix_manager.cpp

    # Utilities
    src/networking/http_client.cpp
)

add_library(TomTomWatch::sdk ALIAS tomtom_sdk)

# ============================================================================
# Include Directories
# ============================================================================
target_include_directories(tomtom_sdk
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ============================================================================
# Dependencies
# ============================================================================

# Link against core library
target_link_libraries(tomtom_sdk
    PUBLIC
    tomtom_core # Public because we expose core types in our API
    PRIVATE
    spdlog::spdlog
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

# ============================================================================
# Compiler Settings
# ============================================================================
target_compile_features(tomtom_sdk PUBLIC cxx_std_17)