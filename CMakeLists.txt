cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

project(Triton LANGUAGES CXX)

cmake_policy(SET CMP0077 NEW)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;")

include(FetchContent)

set(ENABLE_ALL_WARNINGS ON)
set(ENABLE_EFFECTIVE_CXX ON)
include(compiler-options)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

if(APPLE OR UNIX)
   add_compile_options(-stdlib=libc++)
   add_link_options(-fuse-ld=lld -stdlib=libc++)
   set(CMAKE_EXE_LINKER_FLAGS "-stdlib=libc++ -lc++")
endif()

set(CATCH2_VERSION v3.6.0)
set(CEREAL_VERSION v1.3.2)
set(ENTT_VERSION v3.13.2)
set(FUTURES_VERSION v0.1.5)
set(GLFW_VERSION 3.4)
set(GLM_VERSION 0.9.9.8)
set(GLSLANG_VERSION 14.3.0)
set(LIBNOISE_VERSION master)
set(NATIVE_FILE_DIALOG_VERSION v1.2.1)
set(NLOHMANN_JSON_VERSION v3.11.3)
set(OZZ_ANIMATION_VERSION 0.15.0)
set(PLATFORM_FOLDERS_VERSION 4.2.0)
set(SPDLOG_VERSION v1.14.1)
set(TINYGLTF_VERSION v2.9.2)
set(TRACY_VERSION v0.11.1)
set(TROMPELOEIL_VERSION v48)
set(VULKAN_MEMORY_ALLOCATOR_HPP_VERSION master)

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
