project("framework")

set(framework_SRC
  src/FrameworkFactory.cxx
  src/FrameworkContextImpl.cxx
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
  <string>
  <queue>
  PRIVATE
  <memory>
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  BoostDI
  glm
  PRIVATE
  shared
  graphics-vk
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  include
  PRIVATE
  src
)
