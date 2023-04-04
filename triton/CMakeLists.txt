set(name triton)

set(${name}_SRC
   src/core/stb.cxx
   src/core/vma_raii.cxx
   src/core/Log.cxx
   src/game/Game.cxx
   src/game/Application.cxx
   src/game/RenderSystem.cxx
   src/graphics/pipeline/Vertex.cxx
   src/graphics/pipeline/SpirvHelper.cxx
   src/graphics/pipeline/DefaultPipeline.cxx
   src/graphics/pipeline/AbstractPipeline.cxx
   src/graphics/renderer/RendererBase.cxx
   src/graphics/renderer/Clear.cxx
   src/graphics/renderer/Finish.cxx
   src/graphics/renderer/ImGuiRenderer.cxx
   src/graphics/renderer/Terrain.cxx
   src/graphics/Instance.cxx
   src/graphics/RenderDevice.cxx
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
   <fmt/format.h>
   <glm/gtc/type_ptr.hpp>
   <glm/gtx/string_cast.hpp>
   <glm/gtx/matrix_decompose.hpp>
   <GLFW/glfw3.h>
   <glslang/SPIRV/GlslangToSpv.h>
   <imgui.h>
   <memory>
   <ranges>
   <set>
   <format>
   <spdlog/sinks/dist_sink.h>
   <spdlog/sinks/msvc_sink.h>
   <spdlog/sinks/stdout_color_sinks.h>
   <spirv_cross.hpp>
   <tiny_gltf.h>
   <vulkan/vulkan_raii.hpp>
   <vulkan-memory-allocator-hpp/vk_mem_alloc.hpp>
   <GLFW/glfw3.h>
   <entt/entt.hpp>
   <stb_image.h>
)

if(MSVC)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
else()
   add_compile_options(-Wall -Wextra -pedantic)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
endif(MSVC)

add_compile_definitions(
   GLFW_INCLUDE_VULKAN=1
   VULKAN_HPP_NO_CONSTRUCTORS
   GLM_FORCE_RADIANS
   GLM_FORCE_DEPTH_ZERO_TO_ONE
   GLM_ENABLE_EXPERIMENTAL
)

if(WIN32)
   add_compile_definitions(
      VK_USE_PLATFORM_WIN32_KHR
   )
elseif(APPLE)
   add_compile_definitions(
      VK_USE_PLATFORM_MACOS_MVK
   )
endif(WIN32)

find_package(spdlog CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(unofficial-vulkan-memory-allocator-hpp CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)
find_package(spirv_cross_cpp CONFIG REQUIRED)

target_link_libraries(${name}
   PRIVATE
   unofficial::VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp
   imgui::imgui
   spirv-cross-core
   spirv-cross-cpp
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
