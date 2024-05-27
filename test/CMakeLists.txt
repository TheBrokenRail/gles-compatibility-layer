cmake_minimum_required(VERSION 3.16.0)
project(gles-compatibility-layer-test)

# Build Library
add_subdirectory(.. gles-compatibility-layer)
target_compile_definitions(gles-compatibility-layer PRIVATE GLES_COMPATIBILITY_LAYER_TESTING)

# Build
add_executable(main src/main.cpp)
target_link_libraries(main gles-compatibility-layer)

# GLFW
find_package(glfw3 3.3 REQUIRED)
target_link_libraries(main glfw)