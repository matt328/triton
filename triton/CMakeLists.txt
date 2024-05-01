project("triton")

set(triton_SRC
   "src/ctx/Context.cxx"
   "src/ctx/GameplayFacade.cxx"

   "src/gfx/Frame.cxx"
   "src/gfx/GraphicsDevice.cxx"
   "src/gfx/PipelineBuilder.cxx"
   "src/gfx/VkContext.cxx"
   "src/gfx/RenderContext.cxx"
   "src/gfx/vma_raii.cxx"
   "src/gfx/geometry/Vertex.cxx"
   "src/gfx/gui/ImguiHelper.cxx"
   "src/gfx/helpers/SpirvHelper.cxx"
   "src/gfx/textures/Texture.cxx"
   "src/gfx/textures/ResourceManager.cxx"

   "src/gp/GameplaySystem.cxx"
   "src/gp/actions/ActionSystem.cxx"
   "src/gp/ecs/system/CameraSystem.cxx"
   "src/gp/ecs/system/RenderDataSystem.cxx"
   "src/gp/ecs/system/TransformSystem.cxx"
   "src/gp/ecs/system/AnimationSystem.cxx"

   "src/gfx/ds/LayoutFactory.cxx"
   "src/gfx/ds/Layout.cxx"
   "src/gfx/ds/DescriptorSet.cxx"
   "src/gfx/ds/DescriptorSetFactory.cxx"

   "src/ct/Heightfield.cxx"
   "src/gfx/geometry/GeometryFactory.cxx"
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
find_package(Tracy CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)
find_package(spirv_cross_cpp CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator-hpp CONFIG REQUIRED)
find_package(unofficial-noise CONFIG REQUIRED)
find_package(unofficial-noiseutils CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

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
   VULKAN_HPP_DISPATCH_LOADER_DYNAMIC
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
   <filesystem>
   <fstream>
   <functional>
   <future>
   <glslang/SPIRV/GlslangToSpv.h>
   <limits>
   <map>
   <mutex>
   <optional>
   <random>
   <ranges>
   <set>
   <sstream>
   <stb_image.h>
   <string>
   <string_view>
   PUBLIC
   "include/util/Logger.hpp"
   <GLFW/glfw3.h>
   <glm/gtc/matrix_transform.hpp>
   <glm/gtc/type_ptr.hpp>
   <glm/gtx/matrix_decompose.hpp>
   <glm/gtx/quaternion.hpp>
   <glm/gtx/string_cast.hpp>
   <imgui.h>
   <imgui_impl_glfw.h>
   <imgui_impl_vulkan.h>
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
   <tracy/Tracy.hpp>
   <tracy/TracyVulkan.hpp>
   <tracy/TracyC.h>
   <vulkan-memory-allocator-hpp/vk_mem_alloc.hpp>
   <vulkan/vulkan_raii.hpp>
)

target_link_libraries(${PROJECT_NAME}
   PUBLIC
   glfw
   GPUOpen::VulkanMemoryAllocator
   ozz_animation
   PRIVATE
   fastgltf::fastgltf
   glslang::glslang
   glslang::glslang-default-resource-limits
   glslang::SPIRV
   imgui::imgui
   spirv-cross-core
   spirv-cross-cpp
   Tracy::TracyClient
   unofficial::noise::noise
   unofficial::noiseutils::noiseutils
   unofficial::VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp
   ${Vulkan_LIBRARIES}
)

target_include_directories(${PROJECT_NAME}
   PUBLIC
   include
   PRIVATE
   src
   ${Vulkan_INCLUDE_DIRS}
)

add_dependencies(${PROJECT_NAME} assets)
