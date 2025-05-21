project("shared-api")

add_library(${PROJECT_NAME} INTERFACE)

target_precompile_headers(${PROJECT_NAME}
  INTERFACE
  <algorithm>
  <any>
  <fmt/format.h>
  <functional>
  <future>
  <glm/glm.hpp>
  <glm/ext/matrix_clip_space.hpp>
  <glm/gtc/quaternion.hpp>
  <map>
  <memory>
  <optional>
  <ozz/animation/runtime/animation.h>
  <ozz/animation/runtime/local_to_model_job.h>
  <ozz/animation/runtime/sampling_job.h>
  <ozz/animation/runtime/skeleton.h>
  <ozz/base/containers/vector.h>
  <ozz/base/maths/simd_math.h>
  <ozz/base/maths/soa_transform.h>
  <ozz/base/maths/vec_float.h>
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
  asset-forge
)

target_include_directories(${PROJECT_NAME}
  INTERFACE
  include
  ${Vulkan_INCLUDE_DIRS}
)
