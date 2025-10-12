cmake_minimum_required(VERSION 3.15)
project(TomTomWatchManager VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add subdirectories
add_subdirectory(core)
add_subdirectory(cli)