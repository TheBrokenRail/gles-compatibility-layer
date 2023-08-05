# Embed Resources
set(util_list_dir "${CMAKE_CURRENT_LIST_DIR}")
function(embed_resource target file)
    # Get C Name
    get_filename_component(name "${file}" NAME)
    string(MAKE_C_IDENTIFIER "${name}" name)
    # Add Command
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${name}.c"
        COMMAND "${CMAKE_COMMAND}"
        ARGS "-DEMBED_IN=${CMAKE_CURRENT_SOURCE_DIR}/${file}" "-DEMBED_OUT=${CMAKE_CURRENT_BINARY_DIR}/${name}.c" "-P" "${util_list_dir}/embed-resource.cmake"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${file}" "${util_list_dir}/embed-resource.cmake"
    )
    # Add To Target
    target_sources("${target}" PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/${name}.c")
endfunction()
