# =========================================================
# CLI client executable
# =========================================================
add_executable(tomtom_cli
    app/main.cpp
    src/cli_helpers.cpp
    src/commands/device_commands.cpp
    src/commands/activity_commands.cpp
    src/commands/gps_commands.cpp
    src/commands/sync_commands.cpp
    src/commands/utils_commands.cpp
)

# =========================================================
# Include directories
# =========================================================
target_include_directories(tomtom_cli
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# =========================================================
# Link dependencies
# =========================================================
target_link_libraries(tomtom_cli
    PRIVATE
    TomTomWatch::core
    TomTomWatch::sdk
    CLI11::CLI11
)

# =========================================================
# Compiler setup
# =========================================================
target_compile_features(tomtom_cli PUBLIC cxx_std_17)
set_target_properties(tomtom_cli PROPERTIES
    CXX_EXTENSIONS OFF
    OUTPUT_NAME "tomtom-cli"
)

# =========================================================
# Installation (optional, controlled by parent)
# =========================================================
install(TARGETS tomtom_cli
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)