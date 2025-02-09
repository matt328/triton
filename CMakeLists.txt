cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

set(CMAKE_TOOLCHAIN_FILE ./vcpkg/scripts/buildsystems/vcpkg.cmake)

if(POLICY CMP0019)
  cmake_policy(SET CMP0019 NEW)
endif()

project(Triton LANGUAGES CXX)

cmake_policy(SET CMP0077 NEW)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;")

set(ENABLE_ALL_WARNINGS ON)
set(ENABLE_EFFECTIVE_CXX ON)
include(compiler-options)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

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

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(code-coverage)
include(get-boost-di)

add_code_coverage_all_targets()
include(CTest)

add_subdirectory(assets)
add_subdirectory(libs/util-lib)
add_subdirectory(libs/triton)
add_subdirectory(apps/cauldron)
add_subdirectory(apps/testarea)
add_subdirectory(apps/assettool)
add_subdirectory(libs/asset-lib)
add_subdirectory(libs/triton/tests)
