set(name, "models")

set(Models
    viking_room.gltf
    viking_room.bin
)

get_property(GENERATOR_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

set(TO_COPY "${CMAKE_CURRENT_SOURCE_DIR}/$<JOIN:${Models},;${CMAKE_CURRENT_SOURCE_DIR}/>")
set(COPY_TO "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}$<$<BOOL:${GENERATOR_IS_MULTI_CONFIG}>:$<CONFIG>>/assets/models")

set(OUTPUT_AND_DEPENDS "${COPY_TO}/$<JOIN:${Models},;${COPY_TO}/>")

set(OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}assets/models")

add_custom_command(
    OUTPUT "${OUTPUT_DIR}"
    COMMAND ${CMAKE_COMMAND}
    ARGS
    -E make_directory
    ${OUTPUT_DIR}
    DEPENDS ${Models}
)

add_custom_command(
    OUTPUT "${OUTPUT_AND_DEPENDS}"
    COMMAND ${CMAKE_COMMAND}
    ARGS
    -E copy_if_different
    "${TO_COPY}"
    "${COPY_TO}"
    COMMENT "Processing Assets"
    COMMAND_EXPAND_LISTS
    DEPENDS ${OUTPUT_DIR}
)

add_custom_target(models
    DEPENDS "${OUTPUT_AND_DEPENDS}"
)
