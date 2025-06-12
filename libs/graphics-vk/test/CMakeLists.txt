set(test_SRC
  ../src/r3/graph/DebugFrameGraph.cxx
  ../src/r3/graph/PassGraphBuilder.cxx
  DebugFrameGraphTest.cxx
)

add_executable(graphics-vk-test ${test_SRC})

target_compile_definitions(graphics-vk-test
  PRIVATE
  GLM_ENABLE_EXPERIMENTAL
  VULKAN_HPP_NO_CONSTRUCTORS
)

target_precompile_headers(graphics-vk-test
  PRIVATE
  <filesystem>
  <unordered_map>
  <vector>
  <glm/gtc/matrix_transform.hpp>
  <glm/gtc/type_ptr.hpp>
  <glm/gtx/matrix_decompose.hpp>
  <glm/gtx/quaternion.hpp>
  <glm/gtx/string_cast.hpp>
  <cereal/cereal.hpp>
  <cereal/types/vector.hpp>
  <cereal/types/unordered_map.hpp>
  <cereal/types/string.hpp>
  <ozz/animation/runtime/skeleton.h>
  <ozz/base/io/archive.h>
  <catch2/catch_test_macros.hpp>
  <catch2/trompeloeil.hpp>
  <trompeloeil/mock.hpp>
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
)

target_link_libraries(graphics-vk-test
  PUBLIC
  trompeloeil::trompeloeil
  Catch2::Catch2WithMain
  framework
  shared-api
  base-kit

  asset-forge
  glfw
  glm
  imgui_custom
  Tracy::TracyClient
  readerwriterqueue
  ${Vulkan_LIBRARIES}
  VulkanMemoryAllocator-Hpp
  PRIVATE
  glslang
  glslang-default-resource-limits
  libnoise
)

target_include_directories(graphics-vk-test
  PRIVATE
  .
  ../include
  ../src
)

add_custom_target(run_coverage2
  COMMAND ${CMAKE_COMMAND} --build . --target ccov-all-export
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Running code coverage export"
)

include(CTest)
include(Catch)
catch_discover_tests(graphics-vk-test)

target_code_coverage(graphics-vk-test
  AUTO
  ALL
)
