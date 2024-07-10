project("triton")

set(triton_SRC
   "src/Context.cxx"
   "src/GameplayFacade.cxx"

   "src/ct/Heightfield.cxx"
)

add_library(${PROJECT_NAME} STATIC ${triton_SRC})

IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
   target_compile_options(${PROJECT_NAME} PRIVATE ${TRITON_COMPILE_OPTIONS})
ENDIF()

IF(WIN32)
   target_compile_definitions(${PROJECT_NAME}
      PRIVATE
      -D_CRT_SECURE_NO_WARNINGS
      VK_USE_PLATFORM_WIN32_KHR
   )
ENDIF(WIN32)

IF(APPLE)
   target_compile_definitions(${PROJECT_NAME}
      PRIVATE
      VK_USE_PLATFORM_MACOS_MVK
   )
ENDIF(APPLE)

find_package(fastgltf CONFIG REQUIRED FASTGLTF_DISABLE_CUSTOM_MEMORY_POOL FASTGLTF_COMPILE_AS_CPP20=YES)
find_package(glfw3 CONFIG REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(unofficial-noise CONFIG REQUIRED)
find_package(unofficial-noiseutils CONFIG REQUIRED)

find_package(nfd CONFIG REQUIRED)

target_compile_definitions(${PROJECT_NAME}
   PRIVATE
   GLFW_EXPOSE_NATIVE_WIN32
   GLFW_INCLUDE_VULKAN=1
   VULKAN_HPP_NO_CONSTRUCTORS
   GLM_FORCE_RADIANS
   GLM_FORCE_DEPTH_ZERO_TO_ONE
   GLM_ENABLE_EXPERIMENTAL
   NOMINMAX
   VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
   TINYGLTF_NO_STB_IMAGE_WRITE
)

target_precompile_headers(${PROJECT_NAME}
   PRIVATE
   <condition_variable>
   <chrono>
   <entt/entt.hpp>
   <fastgltf/core.hpp>
   <fastgltf/glm_element_traits.hpp>
   <fastgltf/tools.hpp>
   <fastgltf/types.hpp>
   <fastgltf/util.hpp>
   <fstream>
   <functional>
   <future>
   <glslang/SPIRV/GlslangToSpv.h>
   <limits>
   <map>
   <mutex>
   <optional>
   <ozz/animation/runtime/animation.h>
   <ozz/animation/runtime/skeleton.h>
   <random>
   <ranges>
   <set>
   <sstream>
   <stb_image.h>
   <string>
   <string_view>
   PUBLIC
   <filesystem>
   <GLFW/glfw3.h>
   <glm/gtc/matrix_transform.hpp>
   <glm/gtc/type_ptr.hpp>
   <glm/gtx/matrix_decompose.hpp>
   <glm/gtx/quaternion.hpp>
   <glm/gtx/string_cast.hpp>
   <imgui.h>
   <imgui_impl_glfw.h>
   <imgui_impl_vulkan.h>
   <nfd.hpp>
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
   <queue>
   <tiny_gltf.h>
   <tracy/TracyVulkan.hpp>
   <vulkan-memory-allocator-hpp/vk_mem_alloc.hpp>
   <vulkan/vulkan_raii.hpp>
)

target_link_libraries(${PROJECT_NAME}
   PUBLIC
   glfw
   ozz_animation
   common
   graphics
   PRIVATE
   gameplay
   fastgltf::fastgltf
   glslang::glslang
   glslang::glslang-default-resource-limits
   glslang::SPIRV
   imgui::imgui
   nfd::nfd
   unofficial::noise::noise
   unofficial::noiseutils::noiseutils
   ${Vulkan_LIBRARIES}
)

target_include_directories(${PROJECT_NAME}
   PUBLIC
   include
   PRIVATE
   src
   include/tr
   ${Vulkan_INCLUDE_DIRS}
)

add_dependencies(${PROJECT_NAME} assets)

add_subdirectory(common)
add_subdirectory(gameplay)
add_subdirectory(graphics)
