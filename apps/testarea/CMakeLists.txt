project("testarea" VERSION "0.0.1" LANGUAGES CXX)

include(code-coverage)

set(testarea_SRC
   "src/main.cxx"
)

add_executable(${PROJECT_NAME} ${testarea_SRC})

target_code_coverage(${PROJECT_NAME} AUTO)

add_executable(testarea-tests
   "test/TestFactorial.cxx"
   "test/TestFactorial2.cxx"
)

target_code_coverage(testarea-tests AUTO)

include(FetchContent)
FetchContent_Declare(futures
   GIT_REPOSITORY https://github.com/alandefreitas/futures
   GIT_TAG origin/master # or whatever tag you want
)
FetchContent_MakeAvailable(futures)

find_package(trompeloeil CONFIG REQUIRED)
find_package(Catch2 3 REQUIRED)

IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
   target_compile_options(${PROJECT_NAME} PRIVATE ${TRITON_COMPILE_OPTIONS})
ENDIF()

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

include(CTest)
include(Catch)
catch_discover_tests(testarea-tests)
