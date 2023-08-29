set(name game)

set(${name}_SRC
   src/Game.cxx
   src/systems/CameraSystem.cxx
   src/systems/RenderSystem.cxx
   src/systems/TransformSystem.cxx
)

set(${name}_INCLUDE "include")

add_library(${name} STATIC ${game_SRC})

target_precompile_headers(${name} PRIVATE
   <memory>
   <entt/entt.hpp>
   <glm/gtc/type_ptr.hpp>
   <glm/gtx/string_cast.hpp>
   <glm/gtx/matrix_decompose.hpp>
   <glm/gtx/quaternion.hpp>
   <glm/gtc/matrix_transform.hpp>
   <GLFW/glfw3.h>
   <filesystem>
)

find_package(glfw3 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

target_link_libraries(${name}
   PRIVATE
   glfw
   triton
)

target_include_directories(${name}
   PUBLIC
   ${game_INCLUDE}
   PRIVATE
   ${triton_INCLUDE}
   src
)
