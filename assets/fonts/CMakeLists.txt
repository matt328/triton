set(name, "fonts")

set(Fonts
   Roboto-Regular.ttf
)

get_property(GENERATOR_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

set(TO_COPY "${CMAKE_CURRENT_SOURCE_DIR}/$<JOIN:${Fonts},;${CMAKE_CURRENT_SOURCE_DIR}/>")
set(COPY_TO "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}$<$<BOOL:${GENERATOR_IS_MULTI_CONFIG}>:$<CONFIG>>/assets/fonts")

set(OUTPUT_AND_DEPENDS "${COPY_TO}/$<JOIN:${Fonts},;${COPY_TO}/>")

set(OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}assets/fonts")

add_custom_command(
    OUTPUT "${OUTPUT_DIR}"
    COMMAND ${CMAKE_COMMAND}
    ARGS
    -E make_directory
    ${OUTPUT_DIR}
    DEPENDS ${Fonts}
)

add_custom_command(
    OUTPUT "${OUTPUT_AND_DEPENDS}"
    COMMAND ${CMAKE_COMMAND}
    ARGS
    -E copy_if_different
    "${TO_COPY}"
    "${COPY_TO}"
    COMMENT "Processing Fonts"
    COMMAND_EXPAND_LISTS
    DEPENDS ${OUTPUT_DIR}
)

add_custom_target(fonts
    DEPENDS "${OUTPUT_AND_DEPENDS}"
)
