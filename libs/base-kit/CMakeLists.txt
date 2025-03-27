project("base-kit")

set(basekit_SRC
  src/Logger2.cxx
)

add_library(${PROJECT_NAME} STATIC ${basekit_SRC})

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
  <queue>
  <random>
  <string_view>
  <spdlog/spdlog.h>
  <spdlog/async.h>
  <spdlog/async_logger.h>
  <spdlog/sinks/basic_file_sink.h>
  <spdlog/sinks/stdout_color_sinks.h>
  <string>
  <tracy/Tracy.hpp>
  <tracy/TracyC.h>
  <bk/BaseException.hpp>
  <bk/Logger2.hpp>
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  spdlog::spdlog_header_only
  Tracy::TracyClient
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  include
  PRIVATE
  src
)
