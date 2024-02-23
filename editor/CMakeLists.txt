set(editor_SRC
   "main.cxx"
   "Application.cxx"
)
set(project_name "editor")

project("Triton Editor" VERSION "0.0.1" LANGUAGES CXX)

add_executable(${project_name} ${editor_SRC})

IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
   target_compile_options(${project_name} PRIVATE "-Wall" "-Wextra" "-pedantic" "-Wno-nullability-extension" "-Wno-nullability-completeness" "-Wno-deprecated-volatile")
ENDIF()

configure_file(config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

include(FetchContent)

fetchcontent_declare(dynalo
   GIT_REPOSITORY https://github.com/maddouri/dynalo.git
   GIT_TAG 1.0.3
)

fetchcontent_makeavailable(dynalo)

add_compile_definitions(
   TRACY_ENABLE
   GLM_FORCE_RADIANS
   GLM_FORCE_DEPTH_ZERO_TO_ONE
   GLM_ENABLE_EXPERIMENTAL
)

target_precompile_headers(${project_name} PRIVATE
   <entt/entt.hpp>
   <functional>
   <sstream>
)

target_link_libraries(${project_name}
   PRIVATE
   game
   triton
   dynalo
)

target_include_directories(${project_name}
   PUBLIC
   .
)

add_dependencies(${project_name} assets)
