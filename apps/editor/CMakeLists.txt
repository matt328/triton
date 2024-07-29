project("editor" VERSION "0.0.1" LANGUAGES CXX)

set(editor_SRC
   "src/main.cxx"
   "src/Application.cxx"
   "src/ui/Manager.cxx"
   "src/data/DataFacade.cxx"
)

add_executable(${PROJECT_NAME} ${editor_SRC})

configure_file(src/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h)

find_package(cereal CONFIG REQUIRED)
find_package(nfd CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(platform_folders CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

target_compile_definitions(${PROJECT_NAME}
   PRIVATE
   TRACY_ENABLE
   GLM_FORCE_RADIANS
   GLM_FORCE_DEPTH_ZERO_TO_ONE
   GLM_ENABLE_EXPERIMENTAL
)

target_precompile_headers(${PROJECT_NAME}
   PRIVATE
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
   <nlohmann/json.hpp>
   <sago/platform_folders.h>
   <sstream>
   <fstream>
   <imgui.h>
   <imgui_internal.h>
   <imgui_stdlib.h>
   "src/ui/ImGuiStyle.hpp"
   <stb_image.h>
   <glm/gtc/type_ptr.hpp>
   <glm/gtx/string_cast.hpp>
   <mutex>
   <spdlog/details/null_mutex.h>
   <spdlog/sinks/base_sink.h>
   <string>
)

if(WIN32)
   target_link_libraries(${PROJECT_NAME}
      PRIVATE
      dwmapi
      uxtheme
   )
endif()

target_link_libraries(${PROJECT_NAME}
   PRIVATE
   glfw
   cereal::cereal
   triton
   nfd::nfd
   nlohmann_json::nlohmann_json
   sago::platform_folders
   spdlog::spdlog_header_only
   util-lib
)

target_include_directories(${PROJECT_NAME}
   PUBLIC
   .
   src
)

add_dependencies(${PROJECT_NAME} assets)
