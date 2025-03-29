project("framework")

set(framework_SRC
  src/ActionSystem.cxx
  src/DefaultAssetService.cxx
  src/DefaultEventBus.cxx
  src/FixedGameLoop.cxx
  src/ctx/FrameworkContextImpl.cxx
  src/ctx/FrameworkFactory.cxx
  src/GuiCallbackRegistrar.cxx
)

add_library(${PROJECT_NAME} STATIC ${framework_SRC})

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
  <glm/glm.hpp>
  <cereal/archives/portable_binary.hpp>
  <cereal/archives/json.hpp>
  <tracy/Tracy.hpp>
  <tracy/TracyC.h>
  <string>
  <queue>
  PRIVATE
  <memory>
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  asset-forge
  BoostDI
  cereal::cereal
  glm
  Tracy::TracyClient
  shared-api
  base-kit
  game-world
  voxel-terrain
  PRIVATE
  graphics-vk
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  include
  PRIVATE
  src
)
