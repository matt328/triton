project("triton")

set(triton_SRC
   "src/graphics/FrameData.cxx"
   "src/graphics/GraphicsDevice.cxx"
   "src/graphics/ImmediateContext.cxx"
   "src/graphics/Renderer.cxx"
   "src/graphics/vma_raii.cxx"
   "src/graphics/geometry/MeshFactory.cxx"
   "src/graphics/geometry/Vertex.cxx"
   "src/graphics/gui/ImguiHelper.cxx"
   "src/graphics/helpers/SpirvHelper.cxx"
   "src/graphics/textures/Texture.cxx"
   "src/graphics/textures/TextureFactory.cxx"
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
   TINYGLTF_NO_STB_IMAGE_WRITE
)

target_precompile_headers(${PROJECT_NAME}
   PRIVATE
   <entt/entt.hpp>
   <filesystem>
   <fstream>
   <functional>
   <glslang/SPIRV/GlslangToSpv.h>
   <limits>
   <map>
   <optional>
   <ranges>
   <set>
   <stb_image.h>
   <string>
   <string_view>
   <sstream>
   PUBLIC
   "include/util/Logger.hpp"
   <GLFW/glfw3.h>
   <imgui_impl_glfw.h>
   <imgui_impl_vulkan.h>
   <glm/gtc/matrix_transform.hpp>
   <glm/gtc/type_ptr.hpp>
   <glm/gtx/matrix_decompose.hpp>
   <glm/gtx/quaternion.hpp>
   <glm/gtx/string_cast.hpp>
   <imgui.h>
   <vulkan-memory-allocator-hpp/vk_mem_alloc.hpp>
   <vulkan/vulkan_raii.hpp>
   <tracy/Tracy.hpp>
   <tracy/TracyVulkan.hpp>
   <tracy/TracyC.h>
)

target_link_libraries(${PROJECT_NAME}
   PUBLIC
   glfw
   GPUOpen::VulkanMemoryAllocator
   PRIVATE
   glslang::glslang
   glslang::glslang-default-resource-limits
   glslang::SPIRV
   imgui::imgui
   spirv-cross-core
   spirv-cross-cpp
   Tracy::TracyClient
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
