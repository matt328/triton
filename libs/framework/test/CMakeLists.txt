set(test_SRC
  TestStateBuffer.cxx
)

add_executable(framework-test ${test_SRC})

target_compile_definitions(framework-test
  PRIVATE
  TINYGLTF_NO_STB_IMAGE_WRITE
  GLM_ENABLE_EXPERIMENTAL
)

target_precompile_headers(framework-test
  PRIVATE
  <filesystem>
  <unordered_map>
  <vector>
  <glm/gtc/matrix_transform.hpp>
  <glm/gtc/type_ptr.hpp>
  <glm/gtx/matrix_decompose.hpp>
  <glm/gtx/quaternion.hpp>
  <glm/gtx/string_cast.hpp>
  <cereal/cereal.hpp>
  <cereal/types/vector.hpp>
  <cereal/types/unordered_map.hpp>
  <cereal/types/string.hpp>
  <ozz/animation/runtime/skeleton.h>
  <ozz/base/io/archive.h>
  <catch2/catch_test_macros.hpp>
  <catch2/trompeloeil.hpp>
  <trompeloeil/mock.hpp>
)

target_link_libraries(framework-test
  PUBLIC
  trompeloeil::trompeloeil
  Catch2::Catch2WithMain
  framework
  shared-api
  base-kit
)

target_include_directories(framework-test
  PRIVATE
  .
  include
  src
  test
)

add_custom_target(run_coverage
  COMMAND ${CMAKE_COMMAND} --build . --target ccov-all-export
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Running code coverage export"
)

# add_test(NAME CoverageExport
# COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ccov-all-export
# )
include(CTest)
include(Catch)
catch_discover_tests(framework-test)

target_code_coverage(framework-test
  AUTO
  ALL
)
