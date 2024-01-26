set(next_SRC
   "main.cxx"
)
set(project_name "next")

add_executable(${project_name} ${next_SRC})

configure_file(config.h.in config.h)

option(CMAKE_USE_WIN32_THREADS_INIT "using WIN32 threads" ON)

find_package(Threads REQUIRED)

find_package(glfw3 CONFIG REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(Tracy CONFIG REQUIRED)

target_precompile_headers(${project_name} PRIVATE
   "core/Logger.hpp"
)

target_link_libraries(${project_name}
   PRIVATE
   glfw
   glslang::glslang
   glslang::glslang-default-resource-limits
   glslang::SPIRV
   Tracy::TracyClient
   triton
   ${vulkan_LIBRARIES}
)

target_include_directories(${project_name}
   PRIVATE
   "${CMAKE_CURRENT_BINARY_DIR}"
)

add_dependencies(${project_name} assets)
