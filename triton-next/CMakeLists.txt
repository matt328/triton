set(next_SRC
   "main.cxx"

   "game/Game.cxx"
   "game/Application.cxx"
   "game/actions/ActionManager.cxx"
   "game/actions/ActionSet.cxx"
   "game/actions/KeyMap.cxx"
   
   "graphics/FrameData.cxx"
   "graphics/GraphicsDevice.cxx"
   "graphics/ImmediateContext.cxx"
   "graphics/Renderer.cxx"
   "graphics/Vertex.cxx"
   "graphics/vma_raii.cxx"
   "graphics/geometry/MeshFactory.cxx"
   "graphics/helpers/SpirvHelper.cxx"
   "graphics/textures/Texture.cxx"
   "graphics/textures/TextureFactory.cxx"
)
set(project_name "next")

add_executable(${project_name} ${next_SRC})

configure_file(config.h.in config.h)

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

find_package(glfw3 CONFIG REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(Tracy CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)
find_package(spirv_cross_cpp CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator-hpp CONFIG REQUIRED)

target_precompile_headers(${project_name} PRIVATE
   "pch.hpp"
)

target_link_libraries(${project_name}
   PRIVATE
   glfw
   glslang::glslang
   glslang::glslang-default-resource-limits
   glslang::SPIRV
   imgui::imgui
   spirv-cross-core
   spirv-cross-cpp
   Tracy::TracyClient
   unofficial::VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp
   ${vulkan_LIBRARIES}
)

target_include_directories(${project_name}
   PRIVATE
   core
   game
   ${Vulkan_INCLUDE_DIRS}
   "${CMAKE_CURRENT_BINARY_DIR}"
)

add_dependencies(${project_name} assets)
