project("asset-lib")

set(assetlib_SRC
   src/Thing.cxx
)

add_library(${PROJECT_NAME} STATIC ${assetlib_SRC})

find_package(cereal CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

target_compile_definitions(${PROJECT_NAME}
   PRIVATE
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
   <ufbx.h>
)

target_link_libraries(${PROJECT_NAME}
   PUBLIC
   cereal::cereal
   PRIVATE
   glm
   ozz_animation
   ufbx
)

target_include_directories(${PROJECT_NAME}
   PUBLIC
   include
   PRIVATE
   src
)
