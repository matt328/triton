project("asset-forge")

set(assetforge_SRC
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

add_library(${PROJECT_NAME} STATIC ${assetforge_SRC})

target_compile_definitions(${PROJECT_NAME}
  PRIVATE
  TINYGLTF_NO_STB_IMAGE_WRITE
)

target_precompile_headers(${PROJECT_NAME}
  PRIVATE
  <unordered_map>
  <vector>
  PUBLIC
  <filesystem>
  <fstream>
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

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  cereal::cereal
  ozz_animation
  tinygltf
  glm
  PRIVATE
  base-kit
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  include
  PRIVATE
  src
)

target_code_coverage(${PROJECT_NAME})
