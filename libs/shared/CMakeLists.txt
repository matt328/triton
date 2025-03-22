project("shared")

add_library(${PROJECT_NAME} INTERFACE)

target_precompile_headers(${PROJECT_NAME}
  INTERFACE
  <algorithm>
  <fmt/format.h>
  <functional>
  <future>
  <glm/glm.hpp>
  <memory>
  <optional>
  <ozz/animation/runtime/animation.h>
  <ozz/animation/runtime/local_to_model_job.h>
  <ozz/animation/runtime/skeleton.h>
  <ozz/base/maths/soa_transform.h>
  <ozz/base/maths/simd_math.h>
  <ozz/base/maths/soa_transform.h>
  <ozz/base/span.h>
  <ozz/base/io/archive.h>
  <ozz/base/io/stream.h>
  <random>
  <string>
  <tuple>
  <typeindex>
  <variant>
)

target_link_libraries(${PROJECT_NAME}
  INTERFACE
  glm
  ozz_animation
  fmt
)

target_include_directories(${PROJECT_NAME}
  INTERFACE
  include
)
