project("editor" VERSION "0.0.1" LANGUAGES CXX)

set(editor_SRC
   "src/main.cxx"
   "src/Application.cxx"
)

add_executable(${PROJECT_NAME} ${editor_SRC})

message("Windows SDK version: ${CMAKE_SYSTEM_VERSION}")
set(CMAKE_SYSTEM_VERSION 10.0.22000.0)

IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
   target_compile_options(${PROJECT_NAME} PRIVATE ${TRITON_COMPILE_OPTIONS})
ENDIF()

configure_file(src/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h)

target_compile_definitions(${PROJECT_NAME}
   PRIVATE
   TRACY_ENABLE
   GLM_FORCE_RADIANS
   GLM_FORCE_DEPTH_ZERO_TO_ONE
   GLM_ENABLE_EXPERIMENTAL
)

target_precompile_headers(${PROJECT_NAME}
   PRIVATE
   <filesystem>
   <functional>
   <sstream>
   <glm/gtc/type_ptr.hpp>
   <glm/gtx/string_cast.hpp>
   <string>
)

target_link_libraries(${PROJECT_NAME}
   PRIVATE
   triton
   dwmapi
   uxtheme
)

target_include_directories(${PROJECT_NAME}
   PUBLIC
   .
)

add_dependencies(${PROJECT_NAME} assets)
