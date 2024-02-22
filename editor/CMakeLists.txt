set(editor_SRC
   "main.cxx"
   "Application.cxx"
)
set(project_name "editor")

project("Triton Editor" VERSION "0.0.1" LANGUAGES CXX)

add_executable(${project_name} ${editor_SRC})

configure_file(config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

add_compile_definitions(
   TRACY_ENABLE
)

target_precompile_headers(${project_name} PRIVATE
   <functional>
   <sstream>
)

target_link_libraries(${project_name}
   PRIVATE
   triton
)

target_include_directories(${project_name}
   PUBLIC
   .
)

add_dependencies(${project_name} assets)
