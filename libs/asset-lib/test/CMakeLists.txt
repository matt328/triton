set(test_SRC
   TestGltfConverter.cxx
)

add_executable(asset-lib-test ${test_SRC})
target_precompile_headers(asset-lib-test
   PRIVATE
   <filesystem>
   <unordered_map>
   <vector>
   <glm/gtc/matrix_transform.hpp>
   <glm/gtc/type_ptr.hpp>
   <glm/gtx/matrix_decompose.hpp>
   <glm/gtx/quaternion.hpp>
   <glm/gtx/string_cast.hpp>
   <ToString.hpp>
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

target_link_libraries(asset-lib-test
   PRIVATE
   trompeloeil::trompeloeil
   Catch2::Catch2WithMain
   asset-lib
   util-lib
)

target_include_directories(asset-lib-test
   PRIVATE
   .
   include
   include/as
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
catch_discover_tests(asset-lib-test)

target_code_coverage(asset-lib-test AUTO ALL)
