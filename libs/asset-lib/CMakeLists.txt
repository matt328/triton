project("asset-lib")

set(assetlib_SRC
   src/GltfConverter.cxx
)

add_library(${PROJECT_NAME} STATIC ${assetlib_SRC})

find_package(cereal CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")

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
   <cereal/types/unordered_map.hpp>
   <cereal/types/string.hpp>
   <ozz/animation/runtime/skeleton.h>
   <ozz/base/io/archive.h>
)

target_link_libraries(${PROJECT_NAME}
   PUBLIC
   cereal::cereal
   ozz_animation
   PRIVATE
   glm
   util-lib
)

target_include_directories(${PROJECT_NAME}
   PUBLIC
   include
   include/as
   PRIVATE
   src
   ${TINYGLTF_INCLUDE_DIRS}
)
