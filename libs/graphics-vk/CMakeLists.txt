project("graphics-vk")

set(graphicsvk_SRC
  src/VkGraphicsFactory.cxx
  src/ResourceProxyImpl.cxx
  src/VkGraphicsContext.cxx
  src/VkResourceManager.cxx
  src/ImmediateTransferContext.cxx
  src/Window.cxx

  src/dd/DDRenderer.cxx
  src/dd/DrawContext.cxx
  src/dd/DrawContextFactory.cxx
  src/dd/RenderConfigRegistry.cxx
  src/dd/ResourceUploader.cxx
  src/dd/RenderPass.cxx

  src/mem/Allocator.cxx
  src/mem/Buffer.cxx
  src/mem/Image.cxx
  src/mem/ArenaBuffer.cxx
  src/geo/VertexAttributes.cxx
  src/geo/DynamicGeometryData.cxx
  src/geo/StaticGeometryData.cxx
  src/geo/TerrainGeometryData.cxx
  src/DefaultDebugManager.cxx
  src/ImGuiSystem.cxx
  src/NoopDebugManager.cxx
  src/NewRenderContext.cxx
  src/vk/BufferManager.cxx
  src/vk/CommandBufferManager.cxx
  src/vk/ArenaGeometryBuffer.cxx
  src/vk/TextureManager.cxx
  src/vk/core/Context.cxx
  src/vk/core/Instance.cxx
  src/vk/core/Surface.cxx
  src/vk/core/PhysicalDevice.cxx
  src/vk/core/Device.cxx
  src/vk/core/QueueTypes.cxx
  src/vk/core/Swapchain.cxx
  src/vk/sb/DSShaderBinding.cxx
  src/vk/sb/DSShaderBindingFactory.cxx
  src/vk/sb/DSLayoutManager.cxx
  src/vk/sb/DSLayout.cxx
  src/task/DefaultRenderScheduler.cxx
  src/task/Frame.cxx
  src/task/DefaultFrameManager.cxx
  src/task/IndirectRenderTask.cxx
  src/task/RenderTaskFactory.cxx
  src/task/ComputeTask.cxx
  src/task/StaticTask.cxx
  src/task/TerrainTask.cxx
  src/pipeline/IndirectPipeline.cxx
  src/pipeline/StaticPipeline.cxx
  src/pipeline/ComputePipeline.cxx
  src/pipeline/TerrainPipeline.cxx
  src/pipeline/SpirvShaderModuleFactory.cxx

  src/task/debugshapes/DebugTask.cxx
  src/task/debugshapes/DebugPipeline.cxx
)

add_library(${PROJECT_NAME} STATIC ${graphicsvk_SRC})

target_compile_options(${PROJECT_NAME}
  PRIVATE
  $<$<CXX_COMPILER_ID:Clang>:-g -Wall -Wextra -pedantic -Werror>
)

IF(WIN32)
  target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    -D_CRT_SECURE_NO_WARNINGS
  )
ENDIF(WIN32)

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
)

target_precompile_headers(${PROJECT_NAME}
  PUBLIC
  <GLFW/glfw3.h>
  <imgui_impl_glfw.h>
  <imgui_impl_vulkan.h>
  <set>
  <tracy/Tracy.hpp>
  <tracy/TracyC.h>
  <typeindex>
  <variant>
  <vk_mem_alloc.hpp>
  <vulkan/vulkan_raii.hpp>
  <tracy/TracyVulkan.hpp>
  PRIVATE
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  asset-forge
  shared-api
  glfw
  glm
  imgui_custom
  Tracy::TracyClient
  base-kit
  ${Vulkan_LIBRARIES}
  VulkanMemoryAllocator-Hpp
  PRIVATE
  BoostDI
  glslang
  glslang-default-resource-limits
  libnoise
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  include
  PRIVATE
  src
)
