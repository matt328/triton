set(triton_SRC
   "src/Application.cxx"
   "src/game/Game.cxx"

   "src/game/actions/ActionSystem.cxx"

   "src/game/ecs/system/CameraSystem.cxx"
   "src/game/ecs/system/RenderSystem.cxx"

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
set(project_name "triton")

add_library(${project_name} STATIC ${triton_SRC})

add_compile_definitions(
   GLFW_EXPOSE_NATIVE_WIN32
   GLFW_INCLUDE_VULKAN=1
   VULKAN_HPP_NO_CONSTRUCTORS
   GLM_FORCE_RADIANS
   GLM_FORCE_DEPTH_ZERO_TO_ONE
   GLM_ENABLE_EXPERIMENTAL
)

find_package(Threads REQUIRED)
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

target_precompile_headers(${project_name}
   PRIVATE
   <entt/entt.hpp>
   <filesystem>
   <fstream>
   <functional>
   <glm/gtc/matrix_transform.hpp>
   <glm/gtc/type_ptr.hpp>
   <glm/gtx/matrix_decompose.hpp>
   <glm/gtx/quaternion.hpp>
   <glm/gtx/string_cast.hpp>
   <glslang/SPIRV/GlslangToSpv.h>
   <imgui.h>
   <imgui_impl_glfw.h>
   <imgui_impl_vulkan.h>
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
   <vulkan-memory-allocator-hpp/vk_mem_alloc.hpp>
   <vulkan/vulkan_raii.hpp>
   <tracy/Tracy.hpp>
   <tracy/TracyVulkan.hpp>
   <tracy/TracyC.h>
)

target_link_libraries(${project_name}
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

target_include_directories(${project_name}
   PUBLIC
   include
   PRIVATE
   src
   ${Vulkan_INCLUDE_DIRS}
)

add_dependencies(${project_name} assets)
