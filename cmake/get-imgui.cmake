include(FetchContent)

FetchContent_Populate(imgui
   URL https://github.com/ocornut/imgui/archive/refs/tags/v1.91.0-docking.zip
   SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui
)

find_package(Vulkan REQUIRED)

add_library(imgui_custom STATIC
   vendor/imgui/imgui.cpp
   vendor/imgui/imgui_draw.cpp
   vendor/imgui/imgui_demo.cpp
   vendor/imgui/imgui_tables.cpp
   vendor/imgui/imgui_widgets.cpp
   vendor/imgui/misc/cpp/imgui_stdlib.cpp

   vendor/imgui/backends/imgui_impl_glfw.cpp
   vendor/imgui/backends/imgui_impl_vulkan.cpp
)

target_link_libraries(imgui_custom PUBLIC glfw ${Vulkan_LIBRARIES})

target_include_directories(imgui_custom
   PUBLIC
   ${Vulkan_INCLUDE_DIRS}
   ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui
   ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/backends
)
