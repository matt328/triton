set(name triton)

set(${name}_SRC
   src/core/stb.cxx
   src/core/vma_raii.cxx
   src/actions/KeyMap.cxx
   src/Application.cxx
   src/Renderer.cxx
   src/ApplicationContext.cxx
   src/GameContainer.cxx
   src/graphics/pipeline/Vertex.cxx
   src/graphics/pipeline/SpirvHelper.cxx
   src/graphics/pipeline/DefaultPipeline.cxx
   src/graphics/pipeline/AbstractPipeline.cxx
   src/graphics/renderer/Clear.cxx
   src/graphics/renderer/Finish.cxx
   src/graphics/renderer/ImGuiRenderer.cxx
   src/graphics/Instance.cxx
   src/graphics/FrameData.cxx
   src/graphics/ImmediateContext.cxx
   src/graphics/VulkanFactory.cxx
   src/graphics/texture/Texture.cxx
   src/graphics/texture/TextureFactory.cxx
   src/graphics/geometry/MeshFactory.cxx
)

set(${name}_INCLUDE "include")

add_library(${name} STATIC ${triton_SRC})

target_precompile_headers(${name} PRIVATE
   <fstream>
   <functional>
   <glm/gtc/matrix_transform.hpp>
   <glm/gtc/type_ptr.hpp>
   <glm/gtx/matrix_decompose.hpp>
   <glm/gtx/quaternion.hpp>
   <glm/gtx/string_cast.hpp>
   <GLFW/glfw3.h>
   <glslang/SPIRV/GlslangToSpv.h>
   <imgui.h>
   <memory>
   <optional>
   <ostream>
   <sstream>
   <ranges>
   <set>
   <spirv_cross.hpp>
   <stb_image.h>
   <tiny_gltf.h>
   <tracy/Tracy.hpp>
   <tracy/TracyVulkan.hpp>
   <tuple>
   <variant>
   <vulkan-memory-allocator-hpp/vk_mem_alloc.hpp>
   <vulkan/vulkan_raii.hpp>
)

target_precompile_headers(${name} PUBLIC
   <string_view>
   <memory>
)

add_compile_definitions(
   GLFW_INCLUDE_VULKAN=1
   VULKAN_HPP_NO_CONSTRUCTORS
   GLM_FORCE_RADIANS
   GLM_FORCE_DEPTH_ZERO_TO_ONE
   GLM_ENABLE_EXPERIMENTAL
   TRACY_ENABLED
)

option(CMAKE_USE_WIN32_THREADS_INIT "using WIN32 threads" ON)

find_package(Threads REQUIRED)

find_package(Stb REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)
find_package(spirv_cross_cpp CONFIG REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(Tracy CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator-hpp CONFIG REQUIRED)

target_link_libraries(${name}
   PRIVATE
   imgui::imgui
   spirv-cross-core
   spirv-cross-cpp
   glslang::SPIRV
   Tracy::TracyClient
   unofficial::VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp
   ${vulkan_LIBRARIES}
)

target_include_directories(${name}
   PUBLIC
   ${triton_INCLUDE}
   ${utils_INCLUDE}
   ${Vulkan_INCLUDE_DIRS}
   PRIVATE
   src
)
