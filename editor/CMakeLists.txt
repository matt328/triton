set(editor_SRC
   "main.cxx"
)
set(project_name "editor")

project(Triton-Editor VERSION "0.0.1" LANGUAGES CXX)

add_executable(${project_name} ${editor_SRC})

configure_file(config.h.in config.h)

add_compile_definitions(
   TRACY_ENABLE
)

target_precompile_headers(${project_name} PRIVATE
   <functional>
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
