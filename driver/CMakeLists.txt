set(driver_SRC "main.cxx")
set(project_name "driver")

add_executable(${project_name} ${driver_SRC})

if(MSVC)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
else()
   add_compile_options(-Wall -Wextra -pedantic)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
endif(MSVC)

set_property(TARGET ${project_name} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIR}")

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
   triton
   spdlog::spdlog_header_only
   ${vulkan_LIBRARIES}
)

add_dependencies(${project_name} assets)
