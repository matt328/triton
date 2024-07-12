project("graphics")

set(graphics_SRC
   "src/HeightField.cxx"
   "src/Frame.cxx"
   "src/GraphicsDevice.cxx"
   "src/PipelineBuilder.cxx"
   "src/VkContext.cxx"
   "src/RenderContext.cxx"
   "src/geometry/Vertex.cxx"
   "src/gui/ImguiHelper.cxx"
   "src/helpers/SpirvHelper.cxx"
   "src/textures/Texture.cxx"
   "src/textures/ResourceManager.cxx"
   "src/mem/Image.cxx"
   "src/mem/Buffer.cxx"
   "src/mem/Allocator.cxx"
   "src/sb/DSShaderBinding.cxx"
   "src/sb/ShaderBindingFactory.cxx"

   "src/sb/LayoutFactory.cxx"
   "src/sb/Layout.cxx"

   "src/geometry/GeometryFactory.cxx"
)

add_library(${PROJECT_NAME} STATIC ${graphics_SRC})

IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
   target_compile_options(${PROJECT_NAME} PRIVATE ${TRITON_COMPILE_OPTIONS})
ENDIF()

IF(WIN32)
   target_compile_definitions(${PROJECT_NAME}
      PRIVATE
      -D_CRT_SECURE_NO_WARNINGS
   )
ENDIF(WIN32)

find_package(glfw3 CONFIG REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(unofficial-noise CONFIG REQUIRED)
find_package(unofficial-noiseutils CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator-hpp CONFIG REQUIRED)
find_package(Tracy CONFIG REQUIRED)

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
   PRIVATE
   <filesystem>
   <fstream>
   <future>
   <glm/gtc/matrix_transform.hpp>
   <glm/gtc/type_ptr.hpp>
   <glm/gtx/matrix_decompose.hpp>
   <glm/gtx/quaternion.hpp>
   <glm/gtx/string_cast.hpp>
   <glslang/SPIRV/GlslangToSpv.h>
   <imgui.h>
   <queue>
   <set>
   <stb_image.h>
   <vulkan/vulkan_raii.hpp>
   <vulkan-memory-allocator-hpp/vk_mem_alloc.hpp>
   <tracy/Tracy.hpp>
   <tracy/TracyC.h>
   <tracy/TracyVulkan.hpp>
)

target_link_libraries(${PROJECT_NAME}
   PUBLIC
   glfw # Get rid of glfw usage in graphics
   imgui::imgui # Look into externalizing this code
   PRIVATE
   common
   glslang::glslang
   glslang::glslang-default-resource-limits
   glslang::SPIRV
   GPUOpen::VulkanMemoryAllocator
   unofficial::noise::noise
   unofficial::noiseutils::noiseutils
   unofficial::VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp
   Tracy::TracyClient
   ${Vulkan_LIBRARIES}
)

target_include_directories(${PROJECT_NAME}
   PUBLIC
   include
   PRIVATE
   include
   include/gfx
   src
   ${Vulkan_INCLUDE_DIRS}
)
