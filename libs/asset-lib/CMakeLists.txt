project("asset-lib")

set(assetlib_SRC
   src/Thing.cxx
   src/Converter.cxx
   src/GltfConverter.cxx
   src/GltfImporter.cxx
)

add_library(${PROJECT_NAME} STATIC ${assetlib_SRC})

find_package(cereal CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

target_compile_definitions(${PROJECT_NAME}
   PRIVATE
   TINYGLTF_NO_STB_IMAGE_WRITE
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
   <cereal/types/vector.hpp>
   <ozz/animation/runtime/animation.h>
   <ozz/animation/runtime/skeleton.h>
   <ozz/animation/offline/fbx/fbx.h>
   <ozz/animation/offline/fbx/fbx_skeleton.h>
   <ozz/animation/offline/raw_skeleton.h>
   <ozz/animation/offline/skeleton_builder.h>
   <ozz/base/endianness.h>
   <ozz/base/io/archive.h>
   <ozz/animation/runtime/skeleton.h>
   <ozz/base/containers/set.h>
   <ozz/animation/offline/raw_animation_utils.h>
   <ozz/animation/offline/tools/import2ozz.h>
   <ozz/base/containers/map.h>
   <tiny_gltf.h>
   <ufbx.h>
)

target_link_libraries(${PROJECT_NAME}
   PUBLIC
   cereal::cereal
   fbx::sdk
   PRIVATE
   utils-lib
   glm
   ozz_animation
   ufbx
   ozz_animation_fbx
)

target_include_directories(${PROJECT_NAME}
   PUBLIC
   include
   PRIVATE
   src
)
