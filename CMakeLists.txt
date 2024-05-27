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

# GL Version
option(GLES_COMPATIBILITY_LAYER_USE_ES3 "Use OpenGL ES 3" TRUE)
if(GLES_COMPATIBILITY_LAYER_USE_ES3)
    target_compile_definitions(gles-compatibility-layer PUBLIC GLES_COMPATIBILITY_LAYER_USE_ES3)
endif()

# Shaders
include(cmake/util.cmake)
set(SHADER_FOLDER "es2")
if(GLES_COMPATIBILITY_LAYER_USE_ES3)
    set(SHADER_FOLDER "es3")
endif()
embed_resource(gles-compatibility-layer "src/shaders/${SHADER_FOLDER}/main.vsh")
embed_resource(gles-compatibility-layer "src/shaders/${SHADER_FOLDER}/main.fsh")

# Warnings
target_compile_options(gles-compatibility-layer PRIVATE -Wall -Wextra -Werror -Wpointer-arith -Wshadow -Wnull-dereference)