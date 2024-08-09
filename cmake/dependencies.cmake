# Dependency Management

include(FetchContent)

# Catch2
FetchContent_Declare(
   Catch2
   GIT_REPOSITORY https://github.com/catchorg/Catch2.git
   GIT_TAG v3.6.0
   SYSTEM
)
FetchContent_MakeAvailable(Catch2)

# trompeloeil
FetchContent_Declare(
   trompeloeil
   GIT_REPOSITORY https://github.com/rollbear/trompeloeil.git
   GIT_TAG v48
   SYSTEM
)
FetchContent_MakeAvailable(trompeloeil)

# Cereal
set(JUST_INSTALL_CEREAL ON)
FetchContent_Declare(
   Cereal
   GIT_REPOSITORY https://github.com/USCiLab/cereal.git
   GIT_TAG v1.3.2
   SYSTEM
)
FetchContent_MakeAvailable(Cereal)

# EnTT
FetchContent_Declare(
   EnTT
   GIT_REPOSITORY https://github.com/skypjack/entt.git
   GIT_TAG v3.13.2
   SYSTEM
)
FetchContent_MakeAvailable(EnTT)

# futures
FetchContent_Declare(futures
   GIT_REPOSITORY https://github.com/alandefreitas/futures
   GIT_TAG v0.1.5
)
FetchContent_MakeAvailable(futures)

# glfw
FetchContent_Declare(
   glfw3
   GIT_REPOSITORY https://github.com/glfw/glfw.git
   GIT_TAG 3.4
   SYSTEM
)
FetchContent_MakeAvailable(glfw3)

# glm
FetchContent_Declare(
   glm
   GIT_REPOSITORY https://github.com/g-truc/glm.git
   GIT_TAG 0.9.9.8
   SYSTEM
)
FetchContent_MakeAvailable(glm)

# glslang
set(ENABLE_OPT OFF)
FetchContent_Declare(
   glslang
   GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
   GIT_TAG 14.3.0
   SYSTEM
)
FetchContent_MakeAvailable(glslang)

# libnoise
FetchContent_Declare(
   noise
   GIT_REPOSITORY https://github.com/eXpl0it3r/libnoise.git
   GIT_TAG master
   SYSTEM
)
FetchContent_MakeAvailable(noise)

# nativefiledialog-extended
FetchContent_Declare(
   nativefiledialog-extended
   GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended.git
   GIT_TAG v1.2.1
   SYSTEM
)
FetchContent_MakeAvailable(nativefiledialog-extended)

# nlohmann_json
FetchContent_Declare(
   nlohmann_json
   GIT_REPOSITORY https://github.com/nlohmann/json.git
   GIT_TAG v3.11.3
   SYSTEM
)
FetchContent_MakeAvailable(nlohmann_json)

# ozz-animation
set(ozz_build_fbx OFF)
set(ozz_build_samples OFF)
set(ozz_build_howtos OFF)
set(ozz_build_tests OFF)
FetchContent_Declare(
   ozz-animation
   GIT_REPOSITORY https://github.com/guillaumeblanc/ozz-animation.git
   GIT_TAG 0.15.0
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
   GIT_TAG 4.2.0
   SYSTEM
)
FetchContent_MakeAvailable(platform_folders)

# spdlog
FetchContent_Declare(
   spdlog
   GIT_REPOSITORY https://github.com/gabime/spdlog.git
   GIT_TAG v1.14.1
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
   GIT_TAG v2.9.2
   SYSTEM
)
FetchContent_MakeAvailable(tinygltf)

# tracy
FetchContent_Declare(
   tracy
   GIT_REPOSITORY https://github.com/wolfpld/tracy.git
   GIT_TAG v0.10
   SYSTEM
)
set(tracy_BINARY_DIR "${CMAKE_BINARY_DIR}/../tracy_build")
FetchContent_MakeAvailable(tracy)

# vulkan-memory-allocator-hpp
FetchContent_Declare(
   vulkan-memory-allocator-hpp
   GIT_REPOSITORY https://github.com/matt328/VulkanMemoryAllocator-Hpp.git
   GIT_TAG master
   SYSTEM
)
FetchContent_MakeAvailable(vulkan-memory-allocator-hpp)
