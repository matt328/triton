project("assettool" VERSION "0.0.1" LANGUAGES CXX)

set(assettool_SRC
   "src/main.cxx"
)

add_executable(${PROJECT_NAME} ${assettool_SRC})

find_package(cereal CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

target_compile_definitions(${PROJECT_NAME}
   PRIVATE
)

target_precompile_headers(${PROJECT_NAME}
   PRIVATE
   <cereal/archives/binary.hpp>
   <fstream>
   <cstdint>
   <glm/fwd.hpp>
   <ozz/animation/runtime/animation.h>
   <ozz/animation/runtime/local_to_model_job.h>
   <ozz/animation/runtime/sampling_job.h>
   <ozz/animation/runtime/skeleton.h>
   <ozz/base/maths/soa_transform.h>
   <ozz/base/maths/simd_math.h>
   <ozz/base/maths/soa_transform.h>
   <ozz/base/containers/vector.h>
   <ozz/base/span.h>
   <ozz/base/io/archive.h>
   <ozz/base/io/stream.h>
   <vector>
   <filesystem>
   <unordered_map>
   <tiny_gltf.h>
)

target_link_libraries(${PROJECT_NAME}
   PRIVATE
   ozz_animation
   util-lib
   asset-lib
)

target_include_directories(${PROJECT_NAME}
   PUBLIC
   .
)
