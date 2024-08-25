add_subdirectory(shaders)

add_custom_target(assets
   DEPENDS shaders
)