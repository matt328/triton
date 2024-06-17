project("editor" VERSION "0.0.1" LANGUAGES CXX)

set(editor_SRC
   "src/main.cxx"
   "src/Application.cxx"
   "src/ProjectFile.cxx"
   "src/ui/Manager.cxx"
)

add_executable(${PROJECT_NAME} ${editor_SRC})

IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
   target_compile_options(${PROJECT_NAME} PRIVATE ${TRITON_COMPILE_OPTIONS})
ENDIF()

configure_file(src/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(platform_folders CONFIG REQUIRED)

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
   <future>
   <functional>
   <nlohmann/json.hpp>
   <sago/platform_folders.h>
   <sstream>
   <fstream>
   <imgui.h>
   <imgui_internal.h>
   "src/ui/ImFileBrowser.hpp"
   "src/ui/ImGuiStyle.hpp"
   <stb_image.h>
   <glm/gtc/type_ptr.hpp>
   <glm/gtx/string_cast.hpp>
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
   utils-lib
   triton
   nlohmann_json::nlohmann_json
   sago::platform_folders
)

target_include_directories(${PROJECT_NAME}
   PUBLIC
   .
   src
)

add_dependencies(${PROJECT_NAME} assets)
