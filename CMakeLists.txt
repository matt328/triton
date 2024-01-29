cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(CMAKE_TOOLCHAIN_FILE ./vcpkg/scripts/buildsystems/vcpkg.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(triton VERSION "0.0.1" LANGUAGES CXX)

configure_file(driver/config.h.in config.h)

add_compile_definitions(
   NOMINMAX
   VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
   TINYGLTF_NO_STB_IMAGE_WRITE
)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_CXX_SCAN_FOR_MODULES OFF)
SET(CMAKE_EXPERIMENTAL_CXX_SCANDEP_SOURCE OFF)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP OFF)

IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
   add_compile_options(-Wall -Wextra -pedantic -Wno-nullability-extension -Wno-nullability-completeness -Wno-deprecated-volatile)
ENDIF()

IF(APPLE)
   include_directories(AFTER ${VULKAN_PATH})
   link_libraries(${Vulkan_LIBRARY} "-framework AppKit" "-framework QuartzCore")
   add_compile_definitions(
      VK_USE_PLATFORM_MACOS_MVK
   )
ENDIF(APPLE)

IF(WIN32)
   link_libraries(${Vulkan_LIBRARY} ${DIRECTFB_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
   add_definitions(-D_CRT_SECURE_NO_WARNINGS)
   add_compile_definitions(
      VK_USE_PLATFORM_WIN32_KHR
   )
ENDIF(WIN32)

IF(LINUX)
   link_libraries(${XCB_LIBRARIES} ${Vulkan_LIBRARY} ${DIRECTFB_LIBRARIES} ${WAYLAND_CLIENT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
ENDIF(LINUX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/")

add_subdirectory(assets)
add_subdirectory(triton-next)
