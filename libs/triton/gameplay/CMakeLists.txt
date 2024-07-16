project("gameplay")

set(gameplay_SRC
   "src/GameWorld.cxx"
   "src/GameplaySystem.cxx"
   "src/EntitySystem.cxx"
   "src/AnimationFactory.cxx"
   "src/ActionSystem.cxx"
   "src/EntitySystem.cxx"

   "src/systems/AnimationSystem.cxx"
   "src/systems/CameraSystem.cxx"
   "src/systems/RenderDataSystem.cxx"
   "src/systems/TransformSystem.cxx"
)

add_library(${PROJECT_NAME} STATIC ${gameplay_SRC})

IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
   target_compile_options(${PROJECT_NAME} PRIVATE ${TRITON_COMPILE_OPTIONS})
ENDIF()

IF(WIN32)
   target_compile_definitions(${PROJECT_NAME}
      PRIVATE
      -D_CRT_SECURE_NO_WARNINGS
   )
ENDIF(WIN32)

find_package(EnTT CONFIG REQUIRED)
find_package(Tracy CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

target_compile_definitions(${PROJECT_NAME}
   PRIVATE
)

target_precompile_headers(${PROJECT_NAME}
   PRIVATE
   <entt/entt.hpp>
   <filesystem>
   <glm/gtc/matrix_transform.hpp>
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
   <shared_mutex>
   <tracy/Tracy.hpp>
   <tracy/TracyC.h>
   <variant>
)

target_link_libraries(${PROJECT_NAME}
   PRIVATE
   util-lib
   EnTT::EnTT
   glm
   Tracy::TracyClient
   common
   ozz_animation
)

target_include_directories(${PROJECT_NAME}
   PUBLIC
   include
   PRIVATE
   include
   include/gp
   src
)

add_subdirectory("../../../vendor/ozz-animation" "ozz-animation" SYSTEM)