project("base-kit")

set(basekit_SRC
  src/Logger2.cxx
  src/Preferences.cxx
)

add_library(${PROJECT_NAME} STATIC ${basekit_SRC})

target_compile_definitions(${PROJECT_NAME}
  PRIVATE
  SPDLOG_ENABLE_THREAD_NAME
)

target_precompile_headers(${PROJECT_NAME}
  PUBLIC
  <cereal/archives/binary.hpp>
  <cereal/types/unordered_map.hpp>
  <cereal/types/string.hpp>
  <chrono>
  <exception>
  <filesystem>
  <fstream>
  <functional>
  <glm/glm.hpp>
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
  cereal::cereal
  glm
  spdlog::spdlog_header_only
  Tracy::TracyClient
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  include
  PRIVATE
  src
)
