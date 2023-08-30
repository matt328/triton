set(testarea_SRC
   main.cxx 
)

set(project_name "testarea")

add_executable(${project_name} ${testarea_SRC})

set_property(TARGET ${project_name} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIR}")

target_link_libraries(${project_name}
   PRIVATE
)

target_include_directories(${project_name}
   PRIVATE

)

add_dependencies(${project_name} assets)
