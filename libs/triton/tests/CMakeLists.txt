set(test_SRC
  MeshBufferTest.cxx
)

add_executable(triton-tests ${test_SRC})

target_link_libraries(triton-tests
  PUBLIC
  triton
  PRIVATE
  BoostDI
  Catch2::Catch2WithMain
)

target_include_directories(triton-tests
  PUBLIC
  .
  src
)

include(CTest)
include(Catch)
enable_testing()
catch_discover_tests(triton-tests
  TEST_PREFIX ""
  TEST_SUFFIX ""
  EXTRA_ARGS --list-tests
)
