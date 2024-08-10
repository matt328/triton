cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(Triton)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;")

include(FetchContent)
include(compiler-options)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/")

# Include code coverage module
include(code-coverage)
include(dependencies)
include(get-imgui)

add_code_coverage_all_targets()
include(CTest)

add_subdirectory(assets)
add_subdirectory(libs/util-lib)
add_subdirectory(libs/triton)
add_subdirectory(apps/editor)
add_subdirectory(apps/testarea)
add_subdirectory(apps/assettool)
add_subdirectory(libs/asset-lib)
