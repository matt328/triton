cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

set(CMAKE_TOOLCHAIN_FILE ./vcpkg/scripts/buildsystems/vcpkg.cmake)

project(Triton)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/")

set(TRITON_COMPILE_OPTIONS
   "-Wall"
   "-Wextra"
   "-pedantic"
   "-Wno-nullability-extension"
   "-Wno-nullability-completeness"
   "-Wno-deprecated-volatile"
)

add_subdirectory(assets)
add_subdirectory("vendor/ozz-animation" "ozz-animation" SYSTEM)
add_subdirectory(libs/triton)
add_subdirectory(apps/editor)
add_subdirectory(apps/testarea)
add_subdirectory(apps/assettool)
add_subdirectory(libs/asset-lib)
add_subdirectory(libs/utils-lib)
