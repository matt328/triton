project("testarea" VERSION "0.0.1" LANGUAGES CXX)

include(code-coverage)

add_code_coverage_all_targets()

set(testarea_SRC
   "src/main.cxx"
)

add_executable(${PROJECT_NAME} ${testarea_SRC})

target_code_coverage(${PROJECT_NAME} AUTO ALL)

add_executable(testarea-tests
   "test/TestFactorial.cxx"
   "test/TestFactorial2.cxx"
)

target_code_coverage(testarea-tests AUTO ALL)

include(FetchContent)
FetchContent_Declare(futures
   GIT_REPOSITORY https://github.com/alandefreitas/futures
   GIT_TAG origin/master # or whatever tag you want
)
FetchContent_MakeAvailable(futures)

find_package(trompeloeil CONFIG REQUIRED)
find_package(Catch2 3 REQUIRED)

target_compile_definitions(${PROJECT_NAME}
   PRIVATE
)

target_precompile_headers(${PROJECT_NAME}
   PRIVATE
   <iostream>
   <chrono>
   <future>
)

target_link_libraries(${PROJECT_NAME}
   PRIVATE
   futures::futures
)

target_link_libraries(testarea-tests
   PRIVATE
   trompeloeil::trompeloeil
   Catch2::Catch2WithMain
)

target_include_directories(testarea-tests
   PUBLIC
   .
   src
)

target_include_directories(${PROJECT_NAME}
   PUBLIC
   .
)

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

include(CTest)
include(Catch)
catch_discover_tests(testarea-tests)
add_dependencies(coverage testarea-tests)
