project("game-world")

set(gameworld_SRC
  src/systems/AnimationSystem.cxx
  src/systems/CameraSystem.cxx
  src/systems/RenderDataSystem.cxx
  src/systems/TransformSystem.cxx

  src/DefaultGameplaySystem.cxx
  src/EntityService.cxx
  src/EntityServiceProvider.cxx
  src/GameObjectProxyImpl.cxx
  src/GameWorldContextImpl.cxx
  src/GameWorldFactory.cxx
)

add_library(${PROJECT_NAME} STATIC ${gameworld_SRC})

target_compile_options(${PROJECT_NAME}
  PRIVATE
  $<$<CXX_COMPILER_ID:Clang>:-g -Wall -Wextra -pedantic -Werror>
)

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
  BoostDI
  EnTT::EnTT
  shared
  asset-lib
  Tracy::TracyClient
  util-lib
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  include
  PRIVATE
  src
)
