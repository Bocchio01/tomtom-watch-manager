# ===================================================
# Core library example
# ===================================================
file(GLOB CORE_EXAMPLE_SOURCES *.cpp)

foreach(example_src ${CORE_EXAMPLE_SOURCES})
    get_filename_component(example_name ${example_src} NAME_WE)
    add_executable(${example_name} ${example_src})
    target_link_libraries(${example_name} PRIVATE TomTomWatch::core)
    target_link_libraries(${example_name} PRIVATE spdlog::spdlog)
    target_compile_definitions(${example_name} PRIVATE SPDLOG_HEADER_ONLY)
    add_test(NAME ${example_name} COMMAND ${example_name})
endforeach()