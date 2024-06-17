project("ufbx" VERSION "0.0.1" LANGUAGES CXX)

set(ufbx_SRC
   "src/ufbx.c"
)

add_library(${PROJECT_NAME} ${ufbx_SRC})

target_compile_definitions(${PROJECT_NAME}
   PRIVATE
)

target_precompile_headers(${PROJECT_NAME}
   PRIVATE
)

target_link_libraries(${PROJECT_NAME}
   PRIVATE
)

target_include_directories(${PROJECT_NAME}
   PRIVATE
   include
)

target_include_directories(${PROJECT_NAME}
   SYSTEM
   INTERFACE
   include
)
