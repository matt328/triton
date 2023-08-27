cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(CMAKE_TOOLCHAIN_FILE ./vcpkg/scripts/buildsystems/vcpkg.cmake)

project(triton)

find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

add_compile_definitions(
   NOMINMAX
   VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
   TINYGLTF_NO_STB_IMAGE_WRITE
)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-nullability-extension -Wno-nullability-completeness -Wno-deprecated-volatile")
ENDIF()

IF(APPLE)
   include_directories(AFTER ${VULKAN_PATH})
   link_libraries(${Vulkan_LIBRARY} "-framework AppKit" "-framework QuartzCore")
ELSE(APPLE)
   link_libraries(${XCB_LIBRARIES} ${Vulkan_LIBRARY} ${DIRECTFB_LIBRARIES} ${WAYLAND_CLIENT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
ENDIF(APPLE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/")

add_subdirectory(game)
add_subdirectory(triton)
add_subdirectory(assets)
add_subdirectory(driver)
