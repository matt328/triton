set(triton_SRC
   "src/graphics/FrameData.cxx"
   "src/graphics/GraphicsDevice.cxx"
   "src/graphics/ImmediateContext.cxx"
   "src/graphics/VkRenderer.cxx"
   "src/graphics/vma_raii.cxx"
   "src/graphics/geometry/MeshFactory.cxx"
   "src/graphics/geometry/Vertex.cxx"
   "src/graphics/gui/ImguiHelper.cxx"
   "src/graphics/helpers/SpirvHelper.cxx"
   "src/graphics/textures/Texture.cxx"
   "src/graphics/textures/TextureFactory.cxx"
)
project(triton)

add_library(${PROJECT_NAME} STATIC ${triton_SRC})

include(FetchContent)

fetchcontent_declare(GLFW
   GIT_REPOSITORY https://github.com/glfw/glfw.git
   GIT_TAG ${glfw_version}
)

fetchcontent_declare(glm
   GIT_REPOSITORY https://github.com/g-truc/glm.git
   GIT_TAG ${glm_version}
)

fetchcontent_declare(EnTT
   GIT_REPOSITORY https://github.com/skypjack/entt.git
   GIT_TAG ${entt_version}
)

fetchcontent_makeavailable(EnTT)
fetchcontent_makeavailable(GLFW)
fetchcontent_makeavailable(glm)

IF(WIN32)
   target_link_libraries(${PROJECT_NAME}
      PRIVATE
      ${Vulkan_LIBRARY}
      ${DIRECTFB_LIBRARIES}
      ${CMAKE_THREAD_LIBS_INIT}
   )
   target_compile_definitions(${PROJECT_NAME}
      PUBLIC
      VK_USE_PLATFORM_WIN32_KHR
   )
ENDIF(WIN32)

IF(APPLE)
   include_directories(AFTER ${VULKAN_PATH})
   link_libraries(${Vulkan_LIBRARY} "-framework AppKit" "-framework QuartzCore")
   add_compile_definitions(
      VK_USE_PLATFORM_MACOS_MVK
   )
ENDIF(APPLE)

IF(LINUX)
   link_libraries(${XCB_LIBRARIES} ${Vulkan_LIBRARY} ${DIRECTFB_LIBRARIES} ${WAYLAND_CLIENT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
ENDIF(LINUX)

IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
   target_compile_options(${PROJECT_NAME}
      PRIVATE
      "-Wall"
      "-Wextra"
      "-pedantic"
      "-Wno-nullability-extension"
      "-Wno-nullability-completeness"
      "-Wno-deprecated-volatile"
   )
ENDIF()

target_compile_definitions(${PROJECT_NAME}
   PRIVATE
   NOMINMAX
   VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
   TINYGLTF_NO_STB_IMAGE_WRITE
   GLFW_EXPOSE_NATIVE_WIN32
   GLFW_INCLUDE_VULKAN=1
   VULKAN_HPP_NO_CONSTRUCTORS
   GLM_FORCE_RADIANS
   GLM_FORCE_DEPTH_ZERO_TO_ONE
   GLM_ENABLE_EXPERIMENTAL
)

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
   EnTT
   glfw
   glm::glm
   GPUOpen::VulkanMemoryAllocator
   imgui::imgui
   PRIVATE
   glslang::glslang
   glslang::glslang-default-resource-limits
   glslang::SPIRV
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
