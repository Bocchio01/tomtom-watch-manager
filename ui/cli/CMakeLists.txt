# ---------------------------------------------------------
# CLI client executable
# ---------------------------------------------------------
add_executable(tomtom_cli
    src/cli_app.cpp
    src/commands.cpp
    app/main.cpp
)

# ---------------------------------------------------------
# Include directories
# ---------------------------------------------------------
target_include_directories(tomtom_cli PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ---------------------------------------------------------
# Link dependencies
# ---------------------------------------------------------
target_link_libraries(tomtom_cli PRIVATE
    TomTomWatch::core
)

# ---------------------------------------------------------
# Compiler setup
# ---------------------------------------------------------
target_compile_features(tomtom_cli PUBLIC cxx_std_17)
set_target_properties(tomtom_cli PROPERTIES
    CXX_EXTENSIONS OFF
    OUTPUT_NAME "tomtom-cli"
)

# ---------------------------------------------------------
# Installation (optional, controlled by parent)
# ---------------------------------------------------------
install(TARGETS tomtom_cli
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)