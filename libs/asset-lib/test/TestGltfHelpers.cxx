#include "MockTinyGltf.hpp"

#include "GltfHelpers.hpp"
#include <catch2/catch_test_macros.hpp>

#include "Model.hpp"

namespace glm {
   std::ostream& operator<<(std::ostream& os, const glm::vec3& v) {
      os << glm::to_string(v);
      return os;
   }
}

TEST_CASE("Gltf Conversion is tested", "[gltf]") {

   const auto identity = glm::identity<glm::mat4>();

   SECTION("parseNodeTransform creates a transform matrix from the given node",
           "[parseNodeTransform]") {

      SECTION("no transforms returns an identity", "[parseNodeTransform]") {
         const NodeNoTransforms mockNode;
         auto result = tr::as::gltf::Helpers::parseNodeTransform(mockNode);
         REQUIRE(result == identity);
      }
      SECTION("translation only returns a translation matrix", "[parseNodeTransform]") {
         const TranslationTransforms xforms;

         auto result2 = tr::as::gltf::Helpers::parseNodeTransform(xforms);
         auto translation = glm::translate(glm::mat4(1.f), glm::vec3{1.f, 1.f, 1.f});
         auto rotation = glm::mat4_cast(glm::identity<glm::quat>());
         auto scale = glm::scale(glm::mat4(1.f), glm::vec3(1.f, 1.f, 1.f));

         auto expected = translation * rotation * scale;

         REQUIRE(result2 == expected);
      }
      SECTION("rotation only returns a rotation matrix", "[parseNodeTransform]") {
         const RotationTransforms xforms;

         auto result = tr::as::gltf::Helpers::parseNodeTransform(xforms);
         auto translation = glm::translate(glm::mat4(1.f), glm::vec3{0.f, 0.f, 0.f});
         auto rotation = glm::mat4_cast(glm::quat(0.0f, 0.7f, 0.7f, 0.f));
         auto scale = glm::scale(glm::mat4(1.f), glm::vec3(1.f, 1.f, 1.f));

         auto expected = translation * rotation * scale;

         REQUIRE(result == expected);
      }

      SECTION("scale only returns a scale matrix", "[parseNodeTransform]") {
         const ScaleTransforms xforms;
         auto result = tr::as::gltf::Helpers::parseNodeTransform(xforms);

         auto translation = glm::translate(glm::mat4(1.f), glm::vec3{0.f, 0.f, 0.f});
         auto rotation = glm::mat4_cast(glm::identity<glm::quat>());
         auto scale = glm::scale(glm::mat4(1.f), glm::vec3(.7f, .7f, .7f));

         auto expected = translation * rotation * scale;

         REQUIRE(result == expected);
      }

      SECTION("matrix just returns the matrix", "[parseNodeTransform]") {
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
   }

   SECTION("createGeometry processes data correctly", "[createGeometry]") {
      tinygltf::Model model;

      tinygltf::BufferView bufferView;
      bufferView.buffer = 0;
      bufferView.byteOffset = 0;
      model.bufferViews.push_back(bufferView);

      SECTION("processes index data correctly", "[createGeometry]") {
         tinygltf::Primitive primitive;
         primitive.indices = 0;

         glm::mat4 transform = glm::mat4(1.0f);
         tr::as::Model tritonModel;

         SECTION("with unsigned int indices") {
            tinygltf::Accessor accessor;
            accessor.componentType = TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT;
            accessor.count = 3;
            accessor.byteOffset = 0;
            accessor.bufferView = 0;
            model.accessors.push_back(accessor);

            tinygltf::Buffer buffer;
            buffer.data = {1, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0};
            model.buffers.push_back(buffer);

            tr::as::gltf::Helpers::createGeometry(model, primitive, transform, tritonModel);

            REQUIRE(tritonModel.indices.size() == 3);
            REQUIRE(tritonModel.indices[0] == 1);
            REQUIRE(tritonModel.indices[1] == 2);
            REQUIRE(tritonModel.indices[2] == 3);
         }

         SECTION("with unsigned short indices") {
            tinygltf::Accessor accessor;
            accessor.componentType = TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT;
            accessor.count = 3;
            accessor.byteOffset = 0;
            accessor.bufferView = 0;
            model.accessors.push_back(accessor);

            tinygltf::Buffer buffer;
            buffer.data = {1, 0, 2, 0, 3, 0};
            model.buffers.push_back(buffer);

            tr::as::gltf::Helpers::createGeometry(model, primitive, transform, tritonModel);

            REQUIRE(tritonModel.indices.size() == 3);
            REQUIRE(tritonModel.indices[0] == 1);
            REQUIRE(tritonModel.indices[1] == 2);
            REQUIRE(tritonModel.indices[2] == 3);
         }

         SECTION("with unsigned byte indices") {
            tinygltf::Accessor accessor;
            accessor.componentType = TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE;
            accessor.count = 3;
            accessor.byteOffset = 0;
            accessor.bufferView = 0;
            model.accessors.push_back(accessor);

            tinygltf::Buffer buffer;
            buffer.data = {1, 2, 3};
            model.buffers.push_back(buffer);

            tr::as::gltf::Helpers::createGeometry(model, primitive, transform, tritonModel);

            REQUIRE(tritonModel.indices.size() == 3);
            REQUIRE(tritonModel.indices[0] == 1);
            REQUIRE(tritonModel.indices[1] == 2);
            REQUIRE(tritonModel.indices[2] == 3);
         }

         SECTION("with unsigned byte indices") {
            tinygltf::Accessor accessor;
            accessor.componentType = -1;
            accessor.count = 3;
            accessor.byteOffset = 0;
            accessor.bufferView = 0;
            model.accessors.push_back(accessor);

            tinygltf::Buffer buffer;
            buffer.data = {1, 2, 3};
            model.buffers.push_back(buffer);

            REQUIRE_THROWS(
                tr::as::gltf::Helpers::createGeometry(model, primitive, transform, tritonModel));
         }
      }
   }
   SECTION("processes vertex attributes correctly", "[createGeometry]") {
      // Arrange
      tinygltf::Model model;
      tinygltf::Primitive primitive;

      int position = 0;

      // Indices
      tinygltf::Accessor indexAccessor;
      indexAccessor.componentType = TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT;
      indexAccessor.count = 3;
      indexAccessor.byteOffset = 0;
      indexAccessor.bufferView = position;
      model.accessors.push_back(indexAccessor);
      primitive.indices = position;

      tinygltf::BufferView indexBufferView;
      indexBufferView.buffer = position;
      indexBufferView.byteOffset = 0;
      model.bufferViews.push_back(indexBufferView);

      tinygltf::Buffer indexBuffer;
      indexBuffer.data = {1, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0};
      model.buffers.push_back(indexBuffer);

      ++position;

      // Vertices Position Attribute
      tinygltf::Accessor vertexAccessor;
      vertexAccessor.count = 3;
      vertexAccessor.byteOffset = 0;
      vertexAccessor.bufferView = position;
      vertexAccessor.componentType = TINYGLTF_PARAMETER_TYPE_FLOAT;
      vertexAccessor.type = TINYGLTF_TYPE_VEC3;
      model.accessors.push_back(vertexAccessor);
      primitive.attributes["POSITION"] = position;

      tinygltf::BufferView vertexBufferView;
      vertexBufferView.byteOffset = 0;
      vertexBufferView.buffer = position;
      model.bufferViews.push_back(vertexBufferView);

      tinygltf::Buffer buffer;
      buffer.data = {0,   0,  128, 63, 0,   0,  0, 64, 0,   0,  64, 64, 0, 0,  128, 64, 0,  0,
                     160, 64, 0,   0,  192, 64, 0, 0,  224, 64, 0,  0,  0, 65, 0,   0,  16, 65};
      model.buffers.push_back(buffer);
      ++position;

      // Vertices Normals
      tinygltf::Accessor normalAccessor;
      normalAccessor.count = 3;
      normalAccessor.byteOffset = 0;
      normalAccessor.bufferView = position;
      normalAccessor.componentType = TINYGLTF_PARAMETER_TYPE_FLOAT;
      normalAccessor.type = TINYGLTF_TYPE_VEC3;
      model.accessors.push_back(normalAccessor);
      primitive.attributes["NORMAL"] = position;

      tinygltf::BufferView normalBufferView;
      normalBufferView.byteOffset = 0;
      normalBufferView.buffer = position;
      model.bufferViews.push_back(normalBufferView);

      tinygltf::Buffer normalBuffer;
      normalBuffer.data = {0,  0, 128, 63,  0,  0, 0, 64,  0,  0, 64, 64, 0,  0, 128, 64, 0, 0, 160,
                           64, 0, 0,   192, 64, 0, 0, 224, 64, 0, 0,  0,  65, 0, 0,   16, 65};
      model.buffers.push_back(normalBuffer);
      position++;

      // Vertices texCoord
      tinygltf::Accessor texCoordAccessor;
      texCoordAccessor.count = 3;
      texCoordAccessor.byteOffset = 0;
      texCoordAccessor.bufferView = position;
      texCoordAccessor.componentType = TINYGLTF_PARAMETER_TYPE_FLOAT;
      texCoordAccessor.type = TINYGLTF_TYPE_VEC2;
      model.accessors.push_back(texCoordAccessor);
      primitive.attributes["TEXCOORD_0"] = position;

      tinygltf::BufferView texCoordBufferView;
      texCoordBufferView.byteOffset = 0;
      texCoordBufferView.buffer = position;
      model.bufferViews.push_back(texCoordBufferView);

      tinygltf::Buffer texCoordBuffer;
      texCoordBuffer.data = {0, 0, 128, 63, 0, 0, 0,   64, 0, 0, 64,  64,
                             0, 0, 128, 64, 0, 0, 160, 64, 0, 0, 192, 64};
      model.buffers.push_back(texCoordBuffer);
      position++;

      auto transform = glm::mat4(1.0f);
      tr::as::Model tritonModel;
      tr::as::gltf::Helpers::createGeometry(model, primitive, transform, tritonModel);

      // Check Vertices Position
      REQUIRE(tritonModel.vertices.size() == 3);
      REQUIRE(tritonModel.vertices[0].pos == glm::vec3(1.0f, 2.0f, 3.0f));
      REQUIRE(tritonModel.vertices[1].pos == glm::vec3(4.0f, 5.0f, 6.0f));
      REQUIRE(tritonModel.vertices[2].pos == glm::vec3(7.0f, 8.0f, 9.0f));

      // Check Vertices Normal
      REQUIRE(tritonModel.vertices[0].normal == glm::vec3(1.0f, 2.0f, 3.0f));
      REQUIRE(tritonModel.vertices[1].normal == glm::vec3(4.0f, 5.0f, 6.0f));
      REQUIRE(tritonModel.vertices[2].normal == glm::vec3(7.0f, 8.0f, 9.0f));

      // Check TexCoord
      REQUIRE(tritonModel.vertices[0].uv == glm::vec2(1.0f, 2.0f));
      REQUIRE(tritonModel.vertices[1].uv == glm::vec2(3.0f, 4.0f));
      REQUIRE(tritonModel.vertices[2].uv == glm::vec2(5.0f, 6.0f));
   }
};
