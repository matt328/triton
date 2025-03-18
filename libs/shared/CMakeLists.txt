project("shared")

set(shared_SRC
  src/AssetManager.cxx
)

add_library(${PROJECT_NAME} STATIC ${shared_SRC})

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
  <bitset>
  <cereal/archives/json.hpp>
  <cereal/archives/portable_binary.hpp>
  <cstddef>
  <filesystem>
  <fstream>
  <format>
  <functional>
  <future>
  <glm/glm.hpp>
  <glm/gtx/string_cast.hpp>
  <glm/detail/qualifier.hpp>
  <map>
  <mutex>
  <random>
  <ranges>
  <set>
  <shared_mutex>
  <stb_image.h>
  <thread>
  <variant>
  <ozz/animation/runtime/sampling_job.h>
  <ozz/base/containers/vector.h>
  PRIVATE
  <ctime>
  <cstdint>
  <ozz/animation/runtime/animation.h>
  <ozz/animation/runtime/local_to_model_job.h>
  <ozz/animation/runtime/skeleton.h>
  <ozz/base/maths/soa_transform.h>
  <ozz/base/maths/simd_math.h>
  <ozz/base/maths/soa_transform.h>
  <ozz/base/span.h>
  <ozz/base/io/archive.h>
  <ozz/base/io/stream.h>
  <queue>
  <unordered_map>
  <vector>
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  asset-lib
  glm
  util-lib
  PRIVATE
  BoostDI
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  include
  PRIVATE
  src
)
