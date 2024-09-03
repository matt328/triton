project("editor" VERSION "0.0.1" LANGUAGES CXX)

set(editor_SRC
   "src/main.cxx"
   "src/Application.cxx"
   "src/ui/Manager.cxx"
   "src/data/DataFacade.cxx"
)

add_executable(${PROJECT_NAME} ${editor_SRC})

find_package(Vulkan REQUIRED)

configure_file(src/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
   target_compile_definitions(${PROJECT_NAME}
      PUBLIC
      TRACY_ENABLE
      BUILD_TYPE_DEBUG
   )
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
   target_compile_definitions(${PROJECT_NAME}
      PUBLIC
      BUILD_TYPE_RELEASE
   )
endif()

target_compile_definitions(${PROJECT_NAME}
   PRIVATE
   GLM_FORCE_RADIANS
   GLM_FORCE_DEPTH_ZERO_TO_ONE
   GLM_ENABLE_EXPERIMENTAL
)

target_precompile_headers(${PROJECT_NAME}
   PRIVATE
   <functional>
   <cereal/archives/binary.hpp>
   <cereal/types/unordered_map.hpp>
   <cereal/types/map.hpp>
   <cereal/types/vector.hpp>
   <cereal/types/string.hpp>
   <cereal/types/complex.hpp>
   <filesystem>
   <future>
   <functional>
   <GLFW/glfw3.h>
   <nfd.hpp>
   <platform_folders.h>
   <sstream>
   <fstream>
   <imgui.h>
   <imgui_internal.h>
   "src/ui/ImGuiStyle.hpp"
   <stb_image.h>
   <glm/gtc/type_ptr.hpp>
   <glm/gtx/string_cast.hpp>
   <mutex>
   <spdlog/details/null_mutex.h>
   <spdlog/sinks/base_sink.h>
   <string>
   <ranges>
)

if(WIN32)
   target_link_libraries(${PROJECT_NAME}
      PRIVATE
      dwmapi
      uxtheme
   )
   set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,/subsystem:windows")
endif()

target_link_libraries(${PROJECT_NAME}
   PRIVATE
   glfw
   cereal::cereal
   triton
   nfd::nfd
   platform_folders
   spdlog::spdlog_header_only
   util-lib
   imgui_custom
   Vulkan::Vulkan
)

target_include_directories(${PROJECT_NAME}
   PUBLIC
   .
   src
)

add_dependencies(${PROJECT_NAME} assets)
