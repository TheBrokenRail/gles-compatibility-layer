cmake_minimum_required(VERSION 3.16.0)
project(gles-compatibility-layer)

# Build
add_library(gles-compatibility-layer STATIC src/state.c src/passthrough.c src/matrix.c src/draw.c)
target_link_libraries(gles-compatibility-layer m)

# Include Path
option(GLES_COMPATIBILITY_LAYER_USE_DEFAULT_INCLUDE_PATH "Use Default Include Path" TRUE)
if(GLES_COMPATIBILITY_LAYER_USE_DEFAULT_INCLUDE_PATH)
    target_include_directories(gles-compatibility-layer PUBLIC include)
endif()

# Shaders
include(cmake/util.cmake)
embed_resource(gles-compatibility-layer "src/shaders/main.vsh")
embed_resource(gles-compatibility-layer "src/shaders/main.fsh")

# Warnings
target_compile_options(gles-compatibility-layer PRIVATE -Wall -Wextra -Werror -Wpointer-arith -Wshadow -Wnull-dereference)