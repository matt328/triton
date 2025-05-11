project("glfw-vulkan")

set(glfwvulkan_SRC
  src/GlfwWindow.cxx
)

add_library(${PROJECT_NAME} STATIC ${glfwvulkan_SRC})

IF(WIN32)
  target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    -D_CRT_SECURE_NO_WARNINGS
  )
ENDIF(WIN32)

target_compile_definitions(${PROJECT_NAME}
  PRIVATE
  GLM_FORCE_RADIANS
  GLM_FORCE_DEPTH_ZERO_TO_ONE
  GLM_ENABLE_EXPERIMENTAL
  GLFW_INCLUDE_VULKAN=1
  NOMINMAX
)

target_precompile_headers(${PROJECT_NAME}
  PUBLIC
  PRIVATE
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  base-kit
  shared-api
  glfw
  PRIVATE
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  include
  PRIVATE
  src
)
