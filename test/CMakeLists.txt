cmake_minimum_required(VERSION 3.16.0)
project(gles-compatibility-layer-test)

# Build Library
set(GLES_COMPATIBILITY_LAYER_USE_SDL FALSE CACHE BOOL "" FORCE)
add_subdirectory(.. gles-compatibility-layer)
target_compile_definitions(gles-compatibility-layer PRIVATE GLES_COMPATIBILITY_LAYER_TESTING)

# Build
add_executable(main src/main.cpp)
target_link_libraries(main gles-compatibility-layer)
