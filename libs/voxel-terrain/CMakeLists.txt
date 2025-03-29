project("voxel-terrain")

set(voxelterrain_SRC
  src/ctx/VoxelTerrainContext.cxx
  src/ctx/VoxelTerrainFactory.cxx
  src/VoxelTerrainSystemProxy.cxx
)

add_library(${PROJECT_NAME} STATIC ${voxelterrain_SRC})

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
  PRIVATE
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  BoostDI
  shared-api
  base-kit
  PRIVATE
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  include
  PRIVATE
  src
)
