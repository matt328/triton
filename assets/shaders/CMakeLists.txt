set(Shaders
   shader.vert
   shader.frag
   terrain.vert
   terrain.frag
   debug.vert
   debug.frag
   skinned.vert
   skinned.frag
)

get_property(GENERATOR_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

set(TO_COPY "${CMAKE_CURRENT_SOURCE_DIR}/$<JOIN:${Shaders},;${CMAKE_CURRENT_SOURCE_DIR}/>")
set(COPY_TO "${CMAKE_BINARY_DIR}$<$<BOOL:${GENERATOR_IS_MULTI_CONFIG}>:$<CONFIG>>/apps/editor/assets/shaders")

set(OUTPUT_AND_DEPENDS "${COPY_TO}/$<JOIN:${Shaders},;${COPY_TO}/>")

set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/apps/editor/assets/shaders")

add_custom_command(
   OUTPUT "${OUTPUT_DIR}"
   COMMAND ${CMAKE_COMMAND}
   ARGS
   -E make_directory
   ${OUTPUT_DIR}
   DEPENDS ${Shaders}
)

add_custom_command(
   OUTPUT "${OUTPUT_AND_DEPENDS}"
   COMMAND ${CMAKE_COMMAND}
   ARGS
   -E copy_if_different
   "${TO_COPY}"
   "${COPY_TO}"
   COMMENT "Processing Shaders"
   COMMAND_EXPAND_LISTS
   DEPENDS ${OUTPUT_DIR};${Shaders}
)

add_custom_target(shaders
   DEPENDS "${OUTPUT_AND_DEPENDS}"
)
