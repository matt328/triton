project("game")

set(game_SRC
   "src/Game.cxx"
   "src/actions/ActionSystem.cxx"
   "src/ecs/system/CameraSystem.cxx"
   "src/ecs/system/RenderSystem.cxx"
)

add_library(${PROJECT_NAME} STATIC ${game_SRC})

IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
   target_compile_options(${PROJECT_NAME} PRIVATE ${TRITON_COMPILE_OPTIONS})
ENDIF()

find_package(EnTT CONFIG REQUIRED)

target_compile_definitions(${PROJECT_NAME}
   PRIVATE
   GLM_FORCE_RADIANS
   GLM_FORCE_DEPTH_ZERO_TO_ONE
   GLM_ENABLE_EXPERIMENTAL
)

target_precompile_headers(${PROJECT_NAME}
   PRIVATE
   <cstdint>
   <filesystem>
   <functional>
   <variant>
   <entt/entt.hpp>
   PUBLIC
)

target_link_libraries(${PROJECT_NAME}
   PUBLIC
   EnTT::EnTT
   triton
   PRIVATE
)

target_include_directories(${PROJECT_NAME}
   PUBLIC
   include
   PRIVATE
   src
)
