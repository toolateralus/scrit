cmake_minimum_required(VERSION 3.10)
project(scrit)

# Force the use of clang++ as the compiler
set(CMAKE_CXX_COMPILER clang++)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -flto")

# Source files
file(GLOB_RECURSE SRCS src/*.cpp)

# Create object files
add_library(scrit_objects OBJECT ${SRCS})

# Include directories
target_include_directories(scrit_objects PRIVATE include)

# Create executable for debug
add_executable(scrit_debug $<TARGET_OBJECTS:scrit_objects>)
target_compile_definitions(scrit_debug PRIVATE DEBUG)
set_target_properties(scrit_debug PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/debug
)
target_compile_options(scrit_debug PRIVATE -g -O0)
target_include_directories(scrit_debug PRIVATE include)

# Create executable for release
add_executable(scrit_release $<TARGET_OBJECTS:scrit_objects>)
target_compile_definitions(scrit_release PRIVATE NDEBUG)
set_target_properties(scrit_release PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/release
)
target_include_directories(scrit_release PRIVATE include)

# Create static library
add_library(scrit STATIC $<TARGET_OBJECTS:scrit_objects>)
set_target_properties(scrit PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)
target_include_directories(scrit PRIVATE include)
