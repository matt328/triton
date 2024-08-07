cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

set(CMAKE_TOOLCHAIN_FILE ./vcpkg/scripts/buildsystems/vcpkg.cmake)

project(Triton)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/vendor/cmake-scripts;")

include(FetchContent)
include(compiler-options)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/")

include(${CMAKE_CURRENT_SOURCE_DIR}/FetchContent.cmake)

add_subdirectory(assets)
add_subdirectory(libs/util-lib)
add_subdirectory(libs/triton)
add_subdirectory(apps/editor)
add_subdirectory(apps/testarea)
add_subdirectory(apps/assettool)
add_subdirectory(libs/asset-lib)

include(CTest)
