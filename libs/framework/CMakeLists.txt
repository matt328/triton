project("framework")

set(framework_SRC
  src/ActionSystem.cxx
  src/DefaultAssetService.cxx
  src/DefaultEventBus.cxx
  src/ctx/ThreadedFrameworkContext.cxx
  src/GuiCallbackRegistrar.cxx
  src/GeometryGenerator.cxx
  src/RingBuffer.cxx
)

add_library(${PROJECT_NAME} STATIC ${framework_SRC})

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
  cereal::cereal
  glm
  Tracy::TracyClient
  shared-api
  base-kit
  game-world
  voxel-terrain
  PRIVATE
  graphics-vk
  glfw-vulkan
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  include
  PRIVATE
  src
)
