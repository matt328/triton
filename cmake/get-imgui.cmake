include(FetchContent)

set(IMGUI_SRC_DIR ${CMAKE_BINARY_DIR}/_deps/imgui)

FetchContent_Populate(imgui
  URL https://github.com/ocornut/imgui/archive/refs/tags/v1.91.0-docking.zip
  SOURCE_DIR ${IMGUI_SRC_DIR}
)

find_package(Vulkan REQUIRED)

add_library(imgui_custom STATIC
  ${IMGUI_SRC_DIR}/imgui.cpp
  ${IMGUI_SRC_DIR}/imgui_draw.cpp
  ${IMGUI_SRC_DIR}/imgui_demo.cpp
  ${IMGUI_SRC_DIR}/imgui_tables.cpp
  ${IMGUI_SRC_DIR}/imgui_widgets.cpp
  ${IMGUI_SRC_DIR}/misc/cpp/imgui_stdlib.cpp

  ${IMGUI_SRC_DIR}/backends/imgui_impl_glfw.cpp
  ${IMGUI_SRC_DIR}/backends/imgui_impl_vulkan.cpp
)

target_link_libraries(imgui_custom PUBLIC glfw ${Vulkan_LIBRARIES})

target_include_directories(imgui_custom
  PUBLIC
  ${Vulkan_INCLUDE_DIRS}
  ${IMGUI_SRC_DIR}
  ${IMGUI_SRC_DIR}/backends
)

# Suppress warnings for this target
if(MSVC)
  target_compile_options(imgui_custom PRIVATE /w)
else()
  target_compile_options(imgui_custom PRIVATE -w)
endif()