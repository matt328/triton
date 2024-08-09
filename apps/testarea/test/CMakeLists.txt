# Source files for the test target
set(test_SRC
   "TestFactorial.cxx"
   "TestFactorial2.cxx"
)

# Test executable target
add_executable(testarea-tests ${test_SRC})

# Enable code coverage for the test target
target_code_coverage(testarea-tests AUTO ALL)

# Test target settings
target_link_libraries(testarea-tests
   PRIVATE
   trompeloeil::trompeloeil
   Catch2::Catch2WithMain
)

target_include_directories(testarea-tests
   PUBLIC
   .
   ../src
   src
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

# Enable testing and discover tests
include(CTest)
include(Catch)
catch_discover_tests(testarea-tests)

# Ensure coverage target depends on running the tests
add_dependencies(coverage testarea-tests)