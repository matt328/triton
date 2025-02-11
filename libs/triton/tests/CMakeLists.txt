set(test_SRC
  ArenaBufferTest.cxx

  ../modules/common/src/ImGuiAdapter.cxx
  ../modules/triton/src/ComponentFactory.cxx
  ../modules/triton/src/DefaultContext.cxx
  ../modules/triton/src/DefaultEventBus.cxx
  ../modules/gameplay/src/ActionSystem.cxx
  ../modules/gameplay/src/systems/CameraSystem.cxx
  ../modules/gameplay/src/systems/TransformSystem.cxx
  ../modules/gameplay/src/systems/RenderDataSystem.cxx
  ../modules/gameplay/src/systems/AnimationSystem.cxx
  ../modules/gameplay/src/AnimationFactory.cxx
  ../modules/gameplay/src/DefaultGameplaySystem.cxx
  ../modules/gameplay/src/AssetManager.cxx
  ../modules/graphics/src/ImmediateTransferContext.cxx
  ../modules/graphics/src/Window.cxx
  ../modules/graphics/src/mem/Allocator.cxx
  ../modules/graphics/src/mem/Buffer.cxx
  ../modules/graphics/src/mem/Image.cxx
  ../modules/graphics/src/mem/ArenaBuffer.cxx
  ../modules/graphics/src/geo/VertexAttributes.cxx
  ../modules/graphics/src/geo/HeightField.cxx
  ../modules/graphics/src/geo/DynamicGeometryData.cxx
  ../modules/graphics/src/geo/StaticGeometryData.cxx
  ../modules/graphics/src/tex/Texture.cxx
  ../modules/graphics/src/DefaultDebugManager.cxx
  ../modules/graphics/src/ImGuiSystem.cxx
  ../modules/graphics/src/NoopDebugManager.cxx
  ../modules/graphics/src/NewRenderContext.cxx
  ../modules/graphics/src/vk/VkResourceManager.cxx
  ../modules/graphics/src/vk/BufferManager.cxx
  ../modules/graphics/src/vk/CommandBufferManager.cxx
  ../modules/graphics/src/vk/TextureManager.cxx
  ../modules/graphics/src/vk/core/Context.cxx
  ../modules/graphics/src/vk/core/Instance.cxx
  ../modules/graphics/src/vk/core/Surface.cxx
  ../modules/graphics/src/vk/core/PhysicalDevice.cxx
  ../modules/graphics/src/vk/core/Device.cxx
  ../modules/graphics/src/vk/core/QueueTypes.cxx
  ../modules/graphics/src/vk/core/Swapchain.cxx
  ../modules/graphics/src/vk/sb/DSShaderBinding.cxx
  ../modules/graphics/src/vk/sb/DSShaderBindingFactory.cxx
  ../modules/graphics/src/vk/sb/DSLayoutManager.cxx
  ../modules/graphics/src/vk/sb/DSLayout.cxx
  ../modules/graphics/src/vk/ArenaGeometryBuffer.cxx
  ../modules/graphics/src/task/DefaultRenderScheduler.cxx
  ../modules/graphics/src/task/Frame.cxx
  ../modules/graphics/src/task/DefaultFrameManager.cxx
  ../modules/graphics/src/task/IndirectRenderTask.cxx
  ../modules/graphics/src/task/RenderTaskFactory.cxx
  ../modules/graphics/src/task/ComputeTask.cxx
  ../modules/graphics/src/task/StaticTask.cxx
  ../modules/graphics/src/pipeline/IndirectPipeline.cxx
  ../modules/graphics/src/pipeline/StaticPipeline.cxx
  ../modules/graphics/src/pipeline/ComputePipeline.cxx
  ../modules/graphics/src/pipeline/GlslShaderModuleFactory.cxx
)

add_executable(triton-tests ${test_SRC})

target_compile_options(triton-tests
  PRIVATE
  $<$<CXX_COMPILER_ID:Clang>:-g -Wall -Wextra -pedantic -Werror>
)

IF(WIN32)
  target_compile_definitions(triton-tests
    PRIVATE
    -D_CRT_SECURE_NO_WARNINGS
  )
ENDIF(WIN32)

target_code_coverage(triton-tests AUTO ALL)

target_compile_definitions(triton-tests
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

target_precompile_headers(triton-tests
  PUBLIC
  <entt/entt.hpp>
  <GLFW/glfw3.h>
  <bitset>
  <cereal/archives/json.hpp>
  <cereal/archives/portable_binary.hpp>
  <cstddef>
  <filesystem>
  <fstream>
  <format>
  <functional>
  <glm/glm.hpp>
  <glm/gtx/string_cast.hpp>
  <glm/detail/qualifier.hpp>
  <imgui_impl_glfw.h>
  <imgui_impl_vulkan.h>
  <map>
  <mutex>
  <random>
  <set>
  <stb_image.h>
  <thread>
  <tracy/Tracy.hpp>
  <tracy/TracyC.h>
  <typeindex>
  <variant>
  <vma/vk_mem_alloc.h>
  <vulkan-memory-allocator-hpp/vk_mem_alloc.hpp>
  <vulkan/vulkan_raii.hpp>
  <tracy/TracyVulkan.hpp>
  <ozz/animation/runtime/sampling_job.h>
  <ozz/base/containers/vector.h>
  PRIVATE
  <ctime>
  <cstdint>
  <ozz/animation/runtime/animation.h>
  <ozz/animation/runtime/local_to_model_job.h>
  <ozz/animation/runtime/skeleton.h>
  <ozz/base/maths/soa_transform.h>
  <ozz/base/maths/simd_math.h>
  <ozz/base/maths/soa_transform.h>
  <ozz/base/span.h>
  <ozz/base/io/archive.h>
  <ozz/base/io/stream.h>
  <queue>
  <shared_mutex>
  <unordered_map>
  <vector>
)

find_package(Catch2 CONFIG REQUIRED)
find_package(Tracy CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(trompeloeil CONFIG REQUIRED)

target_link_libraries(triton-tests
  PUBLIC
  asset-lib
  EnTT::EnTT
  glfw
  glm
  imgui_custom
  Tracy::TracyClient
  util-lib
  ${Vulkan_LIBRARIES}
  VulkanMemoryAllocator-Hpp
  PRIVATE
  BoostDI
  libnoise
  trompeloeil::trompeloeil
  Catch2::Catch2WithMain
)

if(WIN32)
  target_link_libraries(triton-tests
    PRIVATE
    dwmapi
    uxtheme
  )

  # set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,/subsystem:windows")
endif()

target_include_directories(triton-tests
  PUBLIC
  .
  ../modules/triton/include
  ../modules/common/include
  ../modules/gameplay/include
  ../modules/graphics/include
  PRIVATE
  ../modules/common/src
  ../modules/gameplay/src
  ../modules/graphics/src
  ../modules/triton/include
  ../modules/triton/src
)

add_custom_target(triton-tests-coverage
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ccov-export-triton-tests
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Exporting lcov.info"
)

include(CTest)
include(Catch)
enable_testing()
catch_discover_tests(triton-tests)
