cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

set(CMAKE_TOOLCHAIN_FILE ./vcpkg/scripts/buildsystems/vcpkg.cmake)

project(Triton)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# TODO: move this into each project if needed
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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/")

add_subdirectory(assets)
add_subdirectory(triton)
add_subdirectory(game)
add_subdirectory(editor)
