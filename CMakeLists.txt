cmake_minimum_required(VERSION 3.10)
project(ASCIIConverter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use pkg-config to find OpenCV
find_package(PkgConfig REQUIRED)
pkg_check_modules(OpenCV REQUIRED opencv4)

# Add executable
add_executable(ascii_converter main.cpp)

# Include directories and link libraries using pkg-config
target_include_directories(ascii_converter PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(ascii_converter ${OpenCV_LIBRARIES})

# Enable optimizations for release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(ascii_converter PRIVATE -O3 -march=native)
endif()

# Threading support
find_package(Threads REQUIRED)
target_link_libraries(ascii_converter Threads::Threads)