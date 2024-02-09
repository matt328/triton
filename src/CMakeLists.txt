set(triton_SRC
   "main.cxx"

   "actions/ActionSet.cxx"
   "actions/ActionSystem.cxx"
   "actions/KeyMap.cxx"

   "game/Application.cxx"
   "game/Game.cxx"
      
   "game/ecs/system/CameraSystem.cxx"
   "game/ecs/system/RenderSystem.cxx"

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
set(project_name "triton")

add_executable(${project_name} ${triton_SRC})

configure_file(config.h.in config.h)

add_compile_definitions(
   GLFW_EXPOSE_NATIVE_WIN32
   GLFW_INCLUDE_VULKAN=1
   VULKAN_HPP_NO_CONSTRUCTORS
   GLM_FORCE_RADIANS
   GLM_FORCE_DEPTH_ZERO_TO_ONE
   GLM_ENABLE_EXPERIMENTAL
   TRACY_ENABLED
)

option(CMAKE_USE_WIN32_THREADS_INIT "using WIN32 threads" ON)


option(AUTO_LOCATE_VULKAN "AUTO_LOCATE_VULKAN" ON)

if(AUTO_LOCATE_VULKAN)
	message(STATUS "Attempting auto locate Vulkan using CMake......")
	
	# Find Vulkan Path using CMake's Vulkan Module
	# This will return Boolean 'Vulkan_FOUND' indicating the status of find as success(ON) or fail(OFF).
	# Include directory path - 'Vulkan_INCLUDE_DIRS' and 'Vulkan_LIBRARY' with required libraries.
	find_package(Vulkan)
	
	# Try extracting VulkanSDK path from ${Vulkan_INCLUDE_DIRS}
	if (NOT ${Vulkan_INCLUDE_DIRS} STREQUAL "")
		set(VULKAN_PATH ${Vulkan_INCLUDE_DIRS})
		STRING(REGEX REPLACE "/Include" "" VULKAN_PATH ${VULKAN_PATH})
	endif()
		 
	if(NOT Vulkan_FOUND)
		# CMake may fail to locate the libraries but could be able to 
		# provide some path in Vulkan SDK include directory variable
		# 'Vulkan_INCLUDE_DIRS', try to extract path from this.
		message(STATUS "Failed to locate Vulkan SDK, retrying again...")
		if(EXISTS "${VULKAN_PATH}")
			message(STATUS "Successfully located the Vulkan SDK: ${VULKAN_PATH}")
		else()
			message("Error: Unable to locate Vulkan SDK. Please turn off auto locate option by specifying 'AUTO_LOCATE_VULKAN' as 'OFF'")
			message("and specify manually path using 'VULKAN_SDK' and 'VULKAN_VERSION' variables in the CMakeLists.txt.")
			return()
		endif()
	endif()
else()
	message(STATUS "Attempting to locate Vulkan SDK using manual path......")
	set(VULKAN_SDK "C:/VulkanSDK")
	set(VULKAN_VERSION "1.3.261.1")
	set(VULKAN_PATH "${VULKAN_SDK}/${VULKAN_VERSION}")
	message(STATUS "Using manual specified path: ${VULKAN_PATH}")

	# Check if manual set path exists
	if(NOT EXISTS "${VULKAN_PATH}")
		message("Error: Unable to locate this Vulkan SDK path VULKAN_PATH: ${VULKAN_PATH}, please specify correct path.
		For more information on correct installation process, please refer to subsection 'Getting started with Lunar-G SDK'
		and 'Setting up first project with CMake' in Chapter 3, 'Shaking hands with the device' in this book 'Learning Vulkan', ISBN - 9781786469809.")
	   return()
	endif()
endif()


find_package(Threads REQUIRED)
find_package(unofficial-gainput CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(Tracy CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)
find_package(spirv_cross_cpp CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator-hpp CONFIG REQUIRED)

target_precompile_headers(${project_name} PRIVATE
   "core/Logger.hpp"
   <entt/entt.hpp>
   <filesystem>
   <fstream>
   <functional>
   <gainput/gainput.h>
   <glm/gtc/matrix_transform.hpp>
   <glm/gtc/type_ptr.hpp>
   <glm/gtx/matrix_decompose.hpp>
   <glm/gtx/quaternion.hpp>
   <glm/gtx/string_cast.hpp>
   <glslang/SPIRV/GlslangToSpv.h>
   <map>
   <optional>
   <set>
   <stb_image.h>
   <string>
   <string_view>
   <sstream>
   <GLFW/glfw3.h>
   <GLFW/glfw3native.h>
   <vulkan-memory-allocator-hpp/vk_mem_alloc.hpp>
   <vulkan/vulkan_raii.hpp>
   <tracy/Tracy.hpp>
   <tracy/TracyVulkan.hpp>
   <tracy/TracyC.h>
)

target_link_libraries(${project_name}
   PRIVATE
   unofficial::gainput::gainput
   glfw
   glslang::glslang
   glslang::glslang-default-resource-limits
   glslang::SPIRV
   spirv-cross-core
   spirv-cross-cpp
   Tracy::TracyClient
   GPUOpen::VulkanMemoryAllocator
   unofficial::VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp
   ${Vulkan_LIBRARIES}
)

target_include_directories(${project_name}
   PUBLIC
   .
   ${Vulkan_INCLUDE_DIRS}
)

add_dependencies(${project_name} assets)
