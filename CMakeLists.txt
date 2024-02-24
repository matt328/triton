cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(CMAKE_TOOLCHAIN_FILE ./vcpkg/scripts/buildsystems/vcpkg.cmake)

project(Triton)

set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDES 0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/")

set(entt_version v3.13.1)
set(glfw_version 3.4)
set(glm_version 0.9.9.8)

add_subdirectory(assets)
add_subdirectory(triton)
add_subdirectory(game)
add_subdirectory(editor)
