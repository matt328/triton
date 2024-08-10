#include "MockTinyGltf.hpp"

#include "GltfHelpers.hpp"

TEST_CASE("Gltf Conversion is tested", "[gltf]") {

   // Base
   const NodeNoTransforms mockNode;
   auto result = tr::as::gltf::Helpers::parseNodeTransform(mockNode);
   auto identity = glm::identity<glm::mat4>();
   REQUIRE(result == identity);

   // Translation
   {
      const TranslationTransforms xforms;

      auto result2 = tr::as::gltf::Helpers::parseNodeTransform(xforms);
      auto translation = glm::translate(glm::mat4(1.f), glm::vec3{1.f, 1.f, 1.f});
      auto rotation = glm::mat4_cast(glm::identity<glm::quat>());
      auto scale = glm::scale(glm::mat4(1.f), glm::vec3(1.f, 1.f, 1.f));

      auto expected = translation * rotation * scale;

      REQUIRE(result2 == expected);
   }
   // Rotation
   {
      const RotationTransforms xforms;

      auto result = tr::as::gltf::Helpers::parseNodeTransform(xforms);
      auto translation = glm::translate(glm::mat4(1.f), glm::vec3{0.f, 0.f, 0.f});
      auto rotation = glm::mat4_cast(glm::quat(0.0f, 0.7f, 0.7f, 0.f));
      auto scale = glm::scale(glm::mat4(1.f), glm::vec3(1.f, 1.f, 1.f));

      auto expected = translation * rotation * scale;

      REQUIRE(result == expected);
   }

   // Scale
   {
      const ScaleTransforms xforms;
      auto result = tr::as::gltf::Helpers::parseNodeTransform(xforms);

      auto translation = glm::translate(glm::mat4(1.f), glm::vec3{0.f, 0.f, 0.f});
      auto rotation = glm::mat4_cast(glm::identity<glm::quat>());
      auto scale = glm::scale(glm::mat4(1.f), glm::vec3(.7f, .7f, .7f));

      auto expected = translation * rotation * scale;

      REQUIRE(result == expected);
   }

   // Matrix
   {
      const MatrixTransforms xforms;
      auto result = tr::as::gltf::Helpers::parseNodeTransform(xforms);

      auto expected = glm::mat4(1.0f); // Start with an identity matrix
      for (int i = 0; i < 4; ++i) {
         for (int j = 0; j < 4; ++j) {
            expected[i][j] = 1.0f;
         }
      }

      REQUIRE(result == expected);
   }
};
