set(editor_SRC
   "src/main.cxx"
   "src/Application.cxx"
)

project("editor" VERSION "0.0.1" LANGUAGES CXX)

add_executable(${PROJECT_NAME} ${editor_SRC})

IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
   target_compile_options(${PROJECT_NAME}
      PRIVATE
      "-Wall"
      "-Wextra"
      "-pedantic"
      "-Wno-nullability-extension"
      "-Wno-nullability-completeness"
      "-Wno-deprecated-volatile"
   )
ENDIF()

target_compile_definitions(${PROJECT_NAME}
   PRIVATE
   NOMINMAX
)

configure_file(src/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h)

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

target_precompile_headers(${PROJECT_NAME}
   PRIVATE
   <functional>
   <sstream>
)

target_link_libraries(${PROJECT_NAME}
   PRIVATE
   game
   triton
   dynalo
)

target_include_directories(${PROJECT_NAME}
   PUBLIC
   src
)

add_dependencies(${PROJECT_NAME} assets)
