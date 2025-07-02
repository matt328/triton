project("graphics-vk")

set(graphicsvk_SRC
  src/buffers/ManagedBuffer.cxx
  src/buffers/BufferSystem.cxx

  src/img/ImageManager.cxx

  src/mem/Allocator.cxx
  src/mem/Buffer.cxx
  src/mem/Image.cxx

  src/pipeline/SpirvShaderModuleFactory.cxx

  src/r3/R3Renderer.cxx
  src/r3/GeometryBufferPack.cxx

  src/r3/graph/OrderedFrameGraph.cxx
  src/r3/graph/ResourceAliasRegistry.cxx
  src/r3/graph/barriers/BarrierBuilder.cxx
  src/r3/graph/barriers/BarrierPrecursorGenerator.cxx

  src/r3/render-pass/RenderPassFactory.cxx
  src/r3/render-pass/PipelineFactory.cxx
  src/r3/render-pass/passes/ForwardGraphicsPass.cxx
  src/r3/render-pass/passes/CullingPass.cxx
  src/r3/render-pass/passes/CompositionPass.cxx
  src/r3/render-pass/passes/ImGuiPass.cxx
  src/r3/render-pass/passes/PresentPass.cxx

  src/r3/draw-context/ContextFactory.cxx
  src/r3/draw-context/CullingDispatchContext.cxx
  src/r3/draw-context/ForwardDrawContext.cxx
  src/r3/draw-context/CompositionContext.cxx
  src/r3/draw-context/ImGuiContext.cxx

  src/resources/DefaultAssetSystem.cxx
  src/resources/TransferSystem.cxx
  src/resources/allocators/LinearAllocator.cxx
  src/resources/allocators/GeometryAllocator.cxx
  src/resources/allocators/ArenaAllocator.cxx

  src/task/Frame.cxx
  src/task/DefaultFrameManager.cxx

  src/vk/command-buffer/CommandBufferManager.cxx

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

  src/GraphicsContext.cxx
  src/VkGraphicsContext.cxx
  src/DefaultDebugManager.cxx
  src/NoopDebugManager.cxx
  src/ImageTransitionQueue.cxx
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
  <readerwriterqueue.h>
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
  readerwriterqueue
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

add_subdirectory(test)
