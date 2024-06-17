project("utils-lib")

set(utilslib_SRC
   src/UtilThing.cxx
)

add_library(${PROJECT_NAME} STATIC ${utilslib_SRC})

target_compile_definitions(${PROJECT_NAME}
   PRIVATE
)

target_precompile_headers(${PROJECT_NAME}
   PRIVATE
   PUBLIC
   "include/Logger.hpp"
)

target_link_libraries(${PROJECT_NAME}
   PUBLIC
   PRIVATE
)

target_include_directories(${PROJECT_NAME}
   PUBLIC
   include
   PRIVATE
   src
)
