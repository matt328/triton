set(driver_SRC "main.cxx")
set(project_name "driver")

add_executable(${project_name} ${driver_SRC})

set_property(TARGET ${project_name} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIR}")

configure_file(config.h.in config.h)

find_package(spdlog)
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
   game
   triton
   spdlog::spdlog_header_only
   ${vulkan_LIBRARIES}
)

target_include_directories(${project_name}
   PRIVATE
   "${CMAKE_CURRENT_BINARY_DIR}"
)

add_dependencies(${project_name} assets)
