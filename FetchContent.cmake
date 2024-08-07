# Dependency Management
# Catch2
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.6.0
)
FetchContent_MakeAvailable(Catch2)

# trompeloeil
FetchContent_Declare(
  trompeloeil
  GIT_REPOSITORY https://github.com/rollbear/trompeloeil.git
  GIT_TAG v48
)
FetchContent_MakeAvailable(trompeloeil)

# Cereal
set(JUST_INSTALL_CEREAL ON)
FetchContent_Declare(
  Cereal
  GIT_REPOSITORY https://github.com/USCiLab/cereal.git
  GIT_TAG v1.3.2
)
FetchContent_MakeAvailable(Cereal)

# EnTT
FetchContent_Declare(
  EnTT
  GIT_REPOSITORY https://github.com/skypjack/entt.git
  GIT_TAG v3.13.2
)
FetchContent_MakeAvailable(EnTT)

# glfw
FetchContent_Declare(
  glfw3
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw3)

# glm
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)

# glslang
set(ENABLE_OPT OFF)
FetchContent_Declare(
  glslang
  GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
  GIT_TAG 14.3.0
)
FetchContent_MakeAvailable(glslang)

# libnoise
FetchContent_Declare(
  noise
  GIT_REPOSITORY https://github.com/eXpl0it3r/libnoise.git
  GIT_TAG master
)
FetchContent_MakeAvailable(noise)

# nativefiledialog-extended
FetchContent_Declare(
  nativefiledialog-extended
  GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended.git
  GIT_TAG v1.2.1
)
FetchContent_MakeAvailable(nativefiledialog-extended)

# nlohmann_json
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# platform_folders
FetchContent_Declare(
  platform_folders
  GIT_REPOSITORY https://github.com/sago007/PlatformFolders.git
  GIT_TAG 4.2.0
)
FetchContent_MakeAvailable(platform_folders)

# spdlog
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.14.1
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
)
FetchContent_MakeAvailable(tinygltf)

# tracy
FetchContent_Declare(
  tracy
  GIT_REPOSITORY https://github.com/wolfpld/tracy.git
  GIT_TAG v0.10
)
FetchContent_MakeAvailable(tracy)

# vulkan-memory-allocator-hpp
FetchContent_Declare(
  vulkan-memory-allocator-hpp
  GIT_REPOSITORY https://github.com/YaaZ/VulkanMemoryAllocator-Hpp.git
  GIT_TAG v3.1.0
)
FetchContent_MakeAvailable(vulkan-memory-allocator-hpp)
