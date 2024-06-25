# Set the minimum version of CMake required
cmake_minimum_required(VERSION 3.10)

# Set your project name
project(scrit VERSION 1.0 LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Include directories
include_directories(include)

# Find all source files in the src directory
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Define the executable target based on the source files
add_executable(scrit ${SOURCES})
