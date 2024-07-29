#include "MockTinyGltf.hpp"

#include "GltfHelpers.hpp"

TEST_CASE("Gltf Conversion is tested", "[gltf]") {
   NodeNoTransforms mockNode;

   auto result = tr::as::gltf::Helpers::parseNodeTransform(mockNode);

   auto identity = glm::identity<glm::mat4>();

   REQUIRE(result == identity);
};
