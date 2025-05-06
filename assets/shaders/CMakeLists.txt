set(Shaders
  composition.vert
  composition.frag
  compute.comp
  compute2.comp
  debug.vert
  debug.frag
  dynamic.vert
  dynamic.frag
  shader.vert
  shader.frag
  static.vert
  static.frag
  terrain.vert
  terrain.frag
)

get_property(GENERATOR_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

set(SHADER_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SHADER_BIN_DIR "${CMAKE_BINARY_DIR}$<$<BOOL:${GENERATOR_IS_MULTI_CONFIG}>:$<CONFIG>>/apps/cauldron/assets/shaders")

# Create output directory
add_custom_command(
  OUTPUT "${SHADER_BIN_DIR}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_BIN_DIR}"
)

set(COMPILED_SHADERS)

foreach(SHADER ${Shaders})
  set(SHADER_SOURCE "${SHADER_SRC_DIR}/${SHADER}")
  set(SHADER_BINARY "${SHADER_BIN_DIR}/${SHADER}.spv")

  add_custom_command(
    OUTPUT "${SHADER_BINARY}"
    COMMAND glslangValidator -V "${SHADER_SOURCE}" -o "${SHADER_BINARY}"
    COMMENT "Compiling GLSL shader: ${SHADER} -> SPIR-V"
    DEPENDS "${SHADER_SOURCE}"
  )

  list(APPEND COMPILED_SHADERS "${SHADER_BINARY}")
endforeach()

add_custom_target(shaders
  DEPENDS ${COMPILED_SHADERS}
)