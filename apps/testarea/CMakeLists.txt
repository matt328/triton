cmake_minimum_required(VERSION 3.14)
project("testarea" VERSION "0.0.1" LANGUAGES CXX)

# Include code coverage module
include(code-coverage)

# Enable code coverage for all targets
add_code_coverage_all_targets()

# Source files for the main target
set(testarea_SRC
  "src/main.cxx"
)

# Main executable target
add_executable(${PROJECT_NAME} ${testarea_SRC})

# Enable code coverage for the main target
target_code_coverage(${PROJECT_NAME} AUTO ALL)

# Source files for the test target
set(test_SRC
  "test/TestFactorial.cxx"
  "test/TestFactorial2.cxx"
)

# Test executable target
add_executable(testarea-tests ${test_SRC})

# Enable code coverage for the test target
target_code_coverage(testarea-tests AUTO ALL)

# Fetch futures library
include(FetchContent)
FetchContent_Declare(futures
  GIT_REPOSITORY https://github.com/alandefreitas/futures
  GIT_TAG origin/master # or specify a tag or commit hash
)
FetchContent_MakeAvailable(futures)

# Main target settings
target_compile_definitions(${PROJECT_NAME} PRIVATE)
target_precompile_headers(${PROJECT_NAME} PRIVATE <iostream> <chrono> <future>)
target_link_libraries(${PROJECT_NAME} PRIVATE futures::futures)
target_include_directories(${PROJECT_NAME} PUBLIC .)

# Test target settings
target_link_libraries(testarea-tests PRIVATE trompeloeil::trompeloeil Catch2::Catch2WithMain)
target_include_directories(testarea-tests PUBLIC . src)

# Custom command to generate lcov.info from profdata
add_custom_command(
  OUTPUT lcov.info
  COMMAND llvm-cov export --format=lcov --instr-profile=C:\\Users\\Matt\\Projects\\triton\\build\\ccov\\all-merged.profdata C:\\Users\\Matt\\Projects\\triton\\bin\\testarea-tests.exe > lcov.info
  DEPENDS testarea-tests
  COMMENT "Generating lcov.info from profdata"
)

# Custom target to run the tests and generate lcov.info
add_custom_target(coverage
  DEPENDS lcov.info
  COMMAND ${CMAKE_COMMAND} -E echo "Code coverage report generated: lcov.info"
)

# Enable testing and discover tests
include(CTest)
include(Catch)
catch_discover_tests(testarea-tests)

# Ensure coverage target depends on running the tests
add_dependencies(coverage testarea-tests)