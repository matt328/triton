set(driver_SRC
   "main.cxx"
   "game/GameObject.cxx"
   "game/SceneManager.cxx"
   "game/SceneGame.cxx"
)
set(project_name "driver")

add_executable(${project_name} ${driver_SRC})

set_property(TARGET ${project_name} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIR}")

configure_file(config.h.in config.h)

option(CMAKE_USE_WIN32_THREADS_INIT "using WIN32 threads" ON)

find_package(Threads REQUIRED)

find_package(glfw3 CONFIG REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(Tracy CONFIG REQUIRED)

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
