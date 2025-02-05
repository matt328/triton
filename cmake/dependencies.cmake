# Dependency Management

include(FetchContent)

# Backup original flags
set(old_cxx_flags "${CMAKE_CXX_FLAGS}")
set(old_c_flags "${CMAKE_C_FLAGS}")
set(no_dev_warnings_backup "$CACHE{CMAKE_SUPPRESS_DEVELOPER_WARNINGS}")

# Suppress warnings globally
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /w")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
endif()

set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ON CACHE INTERNAL "" FORCE)

# TLSF
FetchContent_Declare(
  TLSF
  GIT_REPOSITORY https://github.com/mattconte/tlsf.git
  GIT_TAG master
  SYSTEM
)
FetchContent_MakeAvailable(TLSF)

# Catch2
set(CATCH_INSTALL_DOCS OFF)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG ${CATCH2_VERSION}
  SYSTEM
)
FetchContent_MakeAvailable(Catch2)

# Cereal
set(JUST_INSTALL_CEREAL ON)
set(BUILD_DOC OFF)
set(BUILD_SANDBOX OFF)
FetchContent_Declare(
  Cereal
  GIT_REPOSITORY https://github.com/USCiLab/cereal.git
  GIT_TAG ${CEREAL_VERSION}
  SYSTEM
)
FetchContent_MakeAvailable(Cereal)

# EnTT
set(ENTT_USE_LIBCPP ON)
FetchContent_Declare(
  EnTT
  GIT_REPOSITORY https://github.com/skypjack/entt.git
  GIT_TAG ${ENTT_VERSION}
  SYSTEM
)
FetchContent_MakeAvailable(EnTT)

# glfw
set(GLFW_BUILD_WAYLAND OFF)
set(GLFW_BUILD_X11 ON)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
FetchContent_Declare(
  glfw3
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG ${GLFW_VERSION}
  SYSTEM
)
FetchContent_MakeAvailable(glfw3)

# glm
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG ${GLM_VERSION}
  SYSTEM
)
FetchContent_MakeAvailable(glm)

# glslang
set(ENABLE_OPT OFF)
FetchContent_Declare(
  glslang
  GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
  GIT_TAG ${GLSLANG_VERSION}
  SYSTEM
)
FetchContent_MakeAvailable(glslang)

# libnoise
FetchContent_Declare(
  noise
  GIT_REPOSITORY https://github.com/eXpl0it3r/libnoise.git
  GIT_TAG ${LIBNOISE_VERSION}
  SYSTEM
)
FetchContent_MakeAvailable(noise)

# ozz-animation
set(BUILD_SHARED_LIBS OFF)
set(ozz_build_tools OFF)
set(ozz_build_fbx OFF)
set(ozz_build_samples OFF)
set(ozz_build_howtos OFF)
set(ozz_build_tests OFF)
set(ozz_build_msvc_rt_dll ON)
FetchContent_Declare(
  ozz-animation
  GIT_REPOSITORY https://github.com/guillaumeblanc/ozz-animation.git
  GIT_TAG ${OZZ_ANIMATION_VERSION}
  SYSTEM
  CMAKE_ARGS
  -DCMAKE_CXX_FLAGS_INIT=/D_CRT_SECURE_NO_WARNINGS
  -DCMAKE_C_FLAGS_INIT=/D_CRT_SECURE_NO_WARNINGS
)
FetchContent_MakeAvailable(ozz-animation)

# platform_folders
FetchContent_Declare(
  platform_folders
  GIT_REPOSITORY https://github.com/sago007/PlatformFolders.git
  GIT_TAG ${PLATFORM_FOLDERS_VERSION}
  SYSTEM
)
FetchContent_MakeAvailable(platform_folders)

# spdlog
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG ${SPDLOG_VERSION}
  SYSTEM
)
FetchContent_MakeAvailable(spdlog)

# tinygltf
set(TINYGLTF_HEADER_ONLY ON)
set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF)
set(TINYGLTF_BUILD_GL_EXAMPLE OFF)
set(TINYGLTF_BUILD_VALIDATOR_EXAMPLE OFF)
set(TINYGLTF_BUILD_BUILDER_EXAMPLE OFF)
FetchContent_Declare(
  tinygltf
  GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
  GIT_TAG ${TINYGLTF_VERSION}
  CMAKE_ARGS -DCMAKE_POLICY_DEFAULT_CMP0077=NEW
  SYSTEM
)
FetchContent_MakeAvailable(tinygltf)

# tracy
set(TRACY_DELAYED_INIT ON)
FetchContent_Declare(
  tracy
  GIT_REPOSITORY https://github.com/wolfpld/tracy.git
  GIT_TAG ${TRACY_VERSION}
  SYSTEM
)
set(tracy_BINARY_DIR "${CMAKE_BINARY_DIR}/../tracy_build")
FetchContent_MakeAvailable(tracy)

# trompeloeil
FetchContent_Declare(
  trompeloeil
  GIT_REPOSITORY https://github.com/rollbear/trompeloeil.git
  GIT_TAG ${TROMPELOEIL_VERSION}
  SYSTEM
)
FetchContent_MakeAvailable(trompeloeil)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

# vulkan-memory-allocator-hpp
FetchContent_Declare(
  vulkan-memory-allocator-hpp
  GIT_REPOSITORY https://github.com/matt328/VulkanMemoryAllocator-Hpp.git
  GIT_TAG ${VULKAN_MEMORY_ALLOCATOR_HPP_VERSION}
  SYSTEM
)
FetchContent_MakeAvailable(vulkan-memory-allocator-hpp)

# Restore original flags
set(CMAKE_CXX_FLAGS "${old_cxx_flags}")
set(CMAKE_C_FLAGS "${old_c_flags}")
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ${no_dev_warnings_backup} CACHE INTERNAL "" FORCE)
