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
   <vulkan/vulkan_raii.hpp>
   <vk_mem_alloc.hpp>
   PRIVATE
   <bitset>
   <filesystem>
   <fstream>
   <future>
   <glm/gtc/matrix_transform.hpp>
   <glm/gtc/type_ptr.hpp>
   <glm/gtx/matrix_decompose.hpp>
   <glm/gtx/quaternion.hpp>
   <glm/gtx/string_cast.hpp>
   <SPIRV/GlslangToSpv.h>
   <imgui.h>
   <queue>
   <set>
   <stb_image.h>
   <tracy/Tracy.hpp>
   <tracy/TracyC.h>
   <tracy/TracyVulkan.hpp>
)

target_link_libraries(${PROJECT_NAME}
   PUBLIC
   asset-lib
   glfw # Get rid of glfw usage in graphics
   imgui_custom
   ${Vulkan_LIBRARIES}
   VulkanMemoryAllocator-Hpp
   PRIVATE
   util-lib
   common
   glslang
   glslang-default-resource-limits
   SPIRV
   libnoise
   Tracy::TracyClient
)

target_include_directories(${PROJECT_NAME}
   PUBLIC
   include
   ${Vulkan_INCLUDE_DIRS}
   PRIVATE
   include
   include/gfx
   src
)
