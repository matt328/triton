project("util-lib")

set(utillib_SRC
   src/Thing.cxx
)

add_library(${PROJECT_NAME} STATIC ${utillib_SRC})

target_compile_definitions(${PROJECT_NAME}
   PRIVATE
)

target_precompile_headers(${PROJECT_NAME}
   PUBLIC
   <chrono>
   <functional>
   <iomanip>
   <iostream>
   <string_view>
   <Logger.hpp>
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
