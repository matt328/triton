project("game-world")

set(gameworld_SRC
  src/EntityManager.cxx
  
  src/ctx/GameWorldContext.cxx

  src/systems2/FinalizerSystem.cxx
  src/systems2/CameraHandler.cxx
  src/systems2/EditorSystem.cxx
)

add_library(${PROJECT_NAME} STATIC ${gameworld_SRC})

IF(WIN32)
  target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    -D_CRT_SECURE_NO_WARNINGS
  )
ENDIF(WIN32)

target_compile_definitions(${PROJECT_NAME}
  PRIVATE
  GLM_FORCE_RADIANS
  GLM_FORCE_DEPTH_ZERO_TO_ONE
  GLM_ENABLE_EXPERIMENTAL
  NOMINMAX
)

target_precompile_headers(${PROJECT_NAME}
  PUBLIC
  <entt/entt.hpp>
  <glm/glm.hpp>
  <glm/ext/matrix_clip_space.hpp>
  <glm/ext/matrix_transform.hpp>
  <shared_mutex>
  <tracy/Tracy.hpp>
  <tracy/TracyC.h>
  <unordered_map>
  <queue>
  <string>
  <vector>
  PRIVATE
  <memory>
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  EnTT::EnTT
  shared-api
  asset-forge
  Tracy::TracyClient
  base-kit
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  include
  PRIVATE
  src
)
