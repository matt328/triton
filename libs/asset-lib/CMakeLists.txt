project("asset-lib")

set(assetlib_SRC
  src/ModelConverter.cxx
  src/gltf/GltfTransformParser.cxx
  src/gltf/GltfNodeParser.cxx
  src/gltf/GltfGeometryExtractor.cxx
  src/gltf/GltfTextureExtractor.cxx
  src/gltf/GltfSkinningDataExtractor.cxx
  src/gltf/GltfModelLoader.cxx
  src/gltf/GltfSkeletonLoader.cxx
  src/gltf/GltfLoaderImpl.cxx
)

add_library(${PROJECT_NAME} STATIC ${assetlib_SRC})

find_package(Ozz REQUIRED)

target_compile_definitions(${PROJECT_NAME}
  PRIVATE
  TINYGLTF_NO_STB_IMAGE_WRITE
  GLM_ENABLE_EXPERIMENTAL
)

target_precompile_headers(${PROJECT_NAME}
  PRIVATE
  <filesystem>
  <unordered_map>
  <vector>
  PUBLIC
  <glm/gtc/matrix_transform.hpp>
  <glm/gtc/type_ptr.hpp>
  <glm/gtx/matrix_decompose.hpp>
  <glm/gtx/quaternion.hpp>
  <glm/gtx/string_cast.hpp>
  <cereal/types/optional.hpp>
  <cereal/types/vector.hpp>
  <cereal/types/unordered_map.hpp>
  <cereal/types/string.hpp>
  <ozz/animation/runtime/skeleton.h>
  <ozz/base/io/archive.h>
)

find_package(cereal CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  cereal::cereal
  glm::glm
  ${OZZ_LIBRARIES}
  PRIVATE
  util-lib
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  include
  ${OZZ_INCLUDE_DIRS}
  PRIVATE
  src
  ${TINYGLTF_INCLUDE_DIRS}
)

target_code_coverage(${PROJECT_NAME})
