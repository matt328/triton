project("util-lib")

set(utillib_SRC
   src/Thing.cxx
   src/Logger2.cxx
)

add_library(${PROJECT_NAME} STATIC ${utillib_SRC})

target_compile_definitions(${PROJECT_NAME}
   PRIVATE
)

target_precompile_headers(${PROJECT_NAME}
   PUBLIC
   <chrono>
   <exception>
   <functional>
   <iomanip>
   <iostream>
   <string_view>
   <spdlog/spdlog.h>
   <spdlog/sinks/stdout_color_sinks.h>
   <string>
   <BaseException.hpp>
   <Logger2.hpp>
)

target_link_libraries(${PROJECT_NAME}
   PUBLIC
   spdlog::spdlog_header_only
   PRIVATE
   BoostDI
)

target_include_directories(${PROJECT_NAME}
   PUBLIC
   include
   PRIVATE
   src
)
