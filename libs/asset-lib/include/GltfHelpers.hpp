#pragma once

#include "Model.hpp"
#include <tiny_gltf.h>

namespace tr::as::gltf::Helpers {

   inline auto parseNodeTransform(const tinygltf::Node& node) {
      // If glft file has a matrix, we should prefer that
      if (node.matrix.size() == 16) {
         const auto floatVec = std::vector<float>{node.matrix.begin(), node.matrix.end()};
         return glm::make_mat4(floatVec.data());
      }
      // If none of these exist, it will end up with an identity matrix
      auto translation = glm::vec3{0.f, 0.f, 0.f};
      auto rotation = glm::identity<glm::quat>();
      auto scale = glm::vec3(1.f, 1.f, 1.f);

      if (node.translation.size() == 3) {
         translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
      }

      if (node.rotation.size() == 4) {
         rotation = glm::quat(static_cast<float>(node.rotation[3]),
                              static_cast<float>(node.rotation[0]),
                              static_cast<float>(node.rotation[1]),
                              static_cast<float>(node.rotation[2]));
      }

      if (node.scale.size() == 3) {
         scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
      }

      const auto translationMatrix = glm::translate(glm::mat4(1.f), translation);
      const auto rotationMatrix = glm::mat4_cast(rotation);
      const auto scaleMatrix = glm::scale(glm::mat4(1.f), scale);

      return translationMatrix * rotationMatrix * scaleMatrix;
   }

   /// Populate the indices and vertices in the tritonModel from the gltf model
   inline void createGeometry(const tinygltf::Model& model,
                              const tinygltf::Primitive& primitive,
                              const glm::mat4& transform,
                              Model& tritonModel) {
      // Load Indices
      std::vector<uint32_t> indices;
      {
         const auto& accessor = model.accessors[primitive.indices];
         const auto& bufferView = model.bufferViews[accessor.bufferView];
         const auto& buffer = model.buffers[bufferView.buffer];

         // glTF supports different component types of indices
         switch (accessor.componentType) {
            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
               auto buf = reinterpret_cast<const uint32_t*>(
                   &buffer.data[accessor.byteOffset + bufferView.byteOffset]);
               for (size_t index = 0; index < accessor.count; index++) {
                  indices.push_back(buf[index]);
               }
               break;
            }
            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
               auto buf = reinterpret_cast<const uint16_t*>(
                   &buffer.data[accessor.byteOffset + bufferView.byteOffset]);
               for (size_t index = 0; index < accessor.count; index++) {
                  indices.push_back(buf[index]);
               }
               break;
            }
            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
               auto buf = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
               for (size_t index = 0; index < accessor.count; index++) {
                  indices.push_back(buf[index]);
               }
               break;
            }
            default:
               std::stringstream err;
               err << "Index component type " << accessor.componentType;
               throw std::runtime_error(err.str());
         }
      }

      {
         std::vector<Vertex> vertices;
         for (const auto& [attribute, value] : primitive.attributes) {
            const auto& accessor = model.accessors[value];

            const auto& vertexCount = accessor.count;
            const auto& view = model.bufferViews[accessor.bufferView];
            const auto dataOffset = accessor.byteOffset + view.byteOffset;
            const auto& buffer = model.buffers[view.buffer];

            // NOLINTNEXTLINE
            const auto& data = reinterpret_cast<const float*>(&buffer.data[dataOffset]);

            vertices.resize(vertexCount);

            if (attribute == "POSITION") {
               for (size_t i = 0; i < vertexCount; i++) {
                  auto vertexPosition = glm::make_vec3(&data[i * 3]);
                  auto tempVec = transform * glm::vec4(vertexPosition, 1.f);
                  vertices[i].pos = glm::vec3(tempVec);
               }
            }
            if (attribute == "NORMAL") {
               for (size_t i = 0; i < vertexCount; i++) {
                  vertices[i].normal = glm::make_vec3(&data[i * 3]);
               }
            }
            if (attribute == "TEXCOORD_0") {
               for (size_t i = 0; i < vertexCount; i++) {
                  vertices[i].uv = glm::make_vec2(&data[i * 2]);
               }
            }
            if (attribute == "JOINTS_0") {
               for (size_t i = 0; i < vertexCount; i++) {
                  const auto& jointData =
                      reinterpret_cast<const uint8_t*>(&buffer.data[dataOffset]);
                  vertices[i].joint0 = glm::make_vec4(&jointData[i * 4]);
               }
            }
            if (attribute == "WEIGHTS_0") {
               for (size_t i = 0; i < vertexCount; i++) {
                  vertices[i].weight0 = glm::make_vec4(&data[i * 4]);
               }
            }
         }

         tritonModel.vertices = std::move(vertices);
         tritonModel.indices = std::move(indices);
      }
   }

   inline auto createTexture(const tinygltf::Model& model,
                             const int textureIndex,
                             Model& tritonModel) {
      if (textureIndex == -1) {
         tinygltf::Image image;

         image.width = 1;
         image.height = 1;
         image.component = 4;
         image.bits = 8;
         image.pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;

         std::vector<unsigned char> imageData(image.width * image.height * image.component);
         imageData[0] = 255; // Red
         imageData[1] = 255; // Green
         imageData[2] = 255; // Blue
         imageData[3] = 255; // Alpha

         image.image = std::move(imageData);
         tritonModel.imageData = ImageData{image.image, image.width, image.height, image.component};
      } else {
         const auto& texture = model.textures[textureIndex];
         const auto& image = model.images[texture.source];
         tritonModel.imageData = ImageData{image.image, image.width, image.height, image.component};
      }
   }

   inline void parseNode(const tinygltf::Model& model, // NOLINT(*-no-recursion)
                         const tinygltf::Node& node,
                         Model& tritonModel) {
      if (node.mesh != -1) {
         const auto nodeTransform = parseNodeTransform(node);
         const auto& mesh = model.meshes[node.mesh];
         for (const auto& primitive : mesh.primitives) {

            createGeometry(model, primitive, nodeTransform, tritonModel);

            const auto materialIndex = primitive.material;
            const auto& material = model.materials[materialIndex];
            const auto& baseColorTextureIndex =
                material.pbrMetallicRoughness.baseColorTexture.index;
            createTexture(model, baseColorTextureIndex, tritonModel);
         }
      }
      // Exit Criteria is node.children is empty
      for (auto& child : node.children) {
         auto& localNode = model.nodes[child];
         parseNode(model, localNode, tritonModel);
      }
   }
}