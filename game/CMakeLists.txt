set(game_SRC
   "src/Game.cxx"
   "src/actions/ActionSystem.cxx"
   "src/ecs/system/CameraSystem.cxx"
   "src/ecs/system/RenderSystem.cxx"
)

set(project_name "game")

add_library(${project_name} STATIC ${game_SRC})

IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
   target_compile_options(${project_name}
      PRIVATE
      "-Wall"
      "-Wextra"
      "-pedantic"
      "-Wno-nullability-extension"
      "-Wno-nullability-completeness"
      "-Wno-deprecated-volatile"
   )
ENDIF()

target_compile_definitions(${project_name}
   PRIVATE
   GLM_FORCE_RADIANS
   GLM_FORCE_DEPTH_ZERO_TO_ONE
   GLM_ENABLE_EXPERIMENTAL
)

find_package(
   EnTT CONFIG REQUIRED
)

target_precompile_headers(${project_name}
   PRIVATE
   <cstdint>
   <entt/entt.hpp>
   <filesystem>
   <functional>
   <variant>
   PUBLIC
)

target_link_libraries(${project_name}
   PUBLIC
   EnTT::EnTT
   triton
   PRIVATE
)

target_include_directories(${project_name}
   PUBLIC
   include
   PRIVATE
   src
)
