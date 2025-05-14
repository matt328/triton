project("graphics-vk")

set(graphicsvk_SRC
  src/r3/R3Renderer.cxx
  src/r3/graph/DebugFrameGraph.cxx
  src/r3/render-pass/RenderPassFactory.cxx
  src/r3/render-pass/PipelineFactory.cxx
  src/r3/render-pass/GraphicsPass.cxx
  src/r3/render-pass/ComputePass.cxx
  src/r3/draw-context/DrawContext.cxx
  src/r3/draw-context/DispatchContext.cxx
  src/r3/draw-context/ContextFactory.cxx

  src/buffers/ManagedBuffer.cxx
  src/buffers/HostVisibleStrategy.cxx
  src/buffers/ArenaStrategy.cxx
  src/buffers/BufferSystem.cxx

  src/GraphicsContext.cxx
  src/ResourceProxyImpl.cxx
  src/VkGraphicsContext.cxx
  src/VkResourceManager.cxx
  src/ImmediateTransferContext.cxx

  src/img/ImageManager.cxx
  src/img/ImageRegistry.cxx

  src/mem/Allocator.cxx
  src/mem/Buffer.cxx
  src/mem/Image.cxx

  src/geo/VertexAttributes.cxx
  src/geo/DynamicGeometryData.cxx
  src/geo/StaticGeometryData.cxx
  src/geo/TerrainGeometryData.cxx
  src/DefaultDebugManager.cxx
  src/NoopDebugManager.cxx
  src/vk/command-buffer/CommandBufferManager.cxx
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
  src/task/Frame.cxx
  src/task/DefaultFrameManager.cxx
  src/pipeline/IndirectPipeline.cxx
  src/pipeline/StaticPipeline.cxx
  src/pipeline/ComputePipeline.cxx
  src/pipeline/TerrainPipeline.cxx
  src/pipeline/SpirvShaderModuleFactory.cxx

  src/task/debugshapes/DebugPipeline.cxx
)

add_library(${PROJECT_NAME} STATIC ${graphicsvk_SRC})

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
  VMA_DEBUG_ALWAYS_VALIDATE=1
)

target_precompile_headers(${PROJECT_NAME}
  PUBLIC
  <GLFW/glfw3.h>
  <imgui_impl_glfw.h>
  <imgui_impl_vulkan.h>
  <ranges>
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
