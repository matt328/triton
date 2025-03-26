cmake_minimum_required(VERSION 3.14)
project("testarea" VERSION "0.0.1" LANGUAGES CXX)

# Source files for the main target
set(testarea_SRC
  src/main.cxx
)

# Main executable target
add_executable(${PROJECT_NAME} ${testarea_SRC})

# Enable code coverage for the main target
target_code_coverage(${PROJECT_NAME} AUTO ALL)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  BoostDI
  base-kit
  tlsf
)

# Main target settings
target_compile_definitions(${PROJECT_NAME}
  PRIVATE
)
target_precompile_headers(${PROJECT_NAME}
  PRIVATE
  <iostream>
  <chrono>
  <future>
)
target_include_directories(${PROJECT_NAME}
  PUBLIC
  .
)
