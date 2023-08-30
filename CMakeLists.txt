cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(CMAKE_TOOLCHAIN_FILE ./vcpkg/scripts/buildsystems/vcpkg.cmake)

project(triton VERSION "0.0.1")

find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

configure_file(driver/config.h.in config.h)

add_compile_definitions(
   NOMINMAX
   VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
   TINYGLTF_NO_STB_IMAGE_WRITE
)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

SET(CMAKE_CXX_STANDARD 20)
if(MSVC)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
else()
   add_compile_options(-Wall -Wextra -pedantic)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
endif(MSVC)

IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-nullability-extension -Wno-nullability-completeness -Wno-deprecated-volatile")
ENDIF()

IF(APPLE)
   include_directories(AFTER ${VULKAN_PATH})
   link_libraries(${Vulkan_LIBRARY} "-framework AppKit" "-framework QuartzCore")
   add_compile_definitions(
      VK_USE_PLATFORM_MACOS_MVK
   )
ELSE(APPLE)
   link_libraries(${XCB_LIBRARIES} ${Vulkan_LIBRARY} ${DIRECTFB_LIBRARIES} ${WAYLAND_CLIENT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
   add_compile_definitions(
      VK_USE_PLATFORM_WIN32_KHR
   )
ENDIF(APPLE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/")

add_subdirectory(game)
add_subdirectory(triton)
add_subdirectory(assets)
add_subdirectory(driver)
add_subdirectory(testarea)
