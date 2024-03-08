add_subdirectory(models)
add_subdirectory(shaders)
add_subdirectory(textures)

add_custom_target(assets
   DEPENDS models;shaders;textures
)