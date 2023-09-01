#include "MeshFactory.hpp"

#include "Logger.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <tiny_gltf.h>

MeshFactory::MeshFactory(vma::raii::Allocator* allocator, ImmediateContext* transferContext) :
    allocator(allocator), transferContext(transferContext) {
}

std::unique_ptr<Mesh<Models::Vertex, uint32_t>> MeshFactory::loadMeshFromGltf(
    const std::string_view& filename) const {
   tinygltf::Model model;

   std::string error, warning;

   if (tinygltf::TinyGLTF gltfContext;
       gltfContext.LoadASCIIFromFile(&model, &error, &warning, filename.data())) {
      const tinygltf::Scene& scene = model.scenes[model.defaultScene > -1 ? model.defaultScene : 0];

      if (scene.nodes.size() > 1) {
         Log::warn << "model " << filename
                   << " contains more than one node, only loading the first one" << std::endl;
      }

      std::vector<Models::Vertex> vertexBuffer;
      std::vector<uint32_t> indexBuffer;
      if (const auto node = model.nodes[scene.nodes[0]]; node.mesh > -1) {
         for (const auto mesh = model.meshes[node.mesh]; const auto& primitive : mesh.primitives) {
            auto vertexStart = static_cast<uint32_t>(vertexBuffer.size());
            const float* positionBuffer = nullptr;
            const float* normalsBuffer = nullptr;
            const float* texCoordsBuffer = nullptr;
            size_t vertexCount = 0;
            if (primitive.attributes.contains("POSITION")) {
               const auto& accessor =
                   model.accessors[primitive.attributes.find("POSITION")->second];
               const auto& view = model.bufferViews[accessor.bufferView];
               positionBuffer = reinterpret_cast<const float*>(
                   &(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
               vertexCount = accessor.count;
            }
            // Get buffer data for vertex normals
            if (primitive.attributes.contains("NORMAL")) {
               const auto& accessor = model.accessors[primitive.attributes.find("NORMAL")->second];
               const auto& view = model.bufferViews[accessor.bufferView];
               normalsBuffer = reinterpret_cast<const float*>(
                   &(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
            }
            // Get buffer data for vertex texture coordinates
            // glTF supports multiple sets, we only load the first one
            if (primitive.attributes.contains("TEXCOORD_0")) {
               const auto& accessor =
                   model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
               const auto& view = model.bufferViews[accessor.bufferView];
               texCoordsBuffer = reinterpret_cast<const float*>(
                   &(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
            }

            // Append data to model's vertex buffer
            for (size_t v = 0; v < vertexCount; v++) {
               Models::Vertex vert{};
               vert.pos = glm::vec4(glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
               vert.normal = glm::normalize(glm::vec3(
                   normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
               vert.uv =
                   texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec3(0.0f);
               vert.color = glm::vec4(1.0f);
               vertexBuffer.push_back(vert);
            }

            const auto& accessor = model.accessors[primitive.indices];
            const auto& bufferView = model.bufferViews[accessor.bufferView];
            const auto& buffer = model.buffers[bufferView.buffer];

            // glTF supports different component types of indices
            switch (accessor.componentType) {
               case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                  auto buf = reinterpret_cast<const uint32_t*>(
                      &buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                  for (size_t index = 0; index < accessor.count; index++) {
                     indexBuffer.push_back(buf[index] + vertexStart);
                  }
                  break;
               }
               case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                  auto buf = reinterpret_cast<const uint16_t*>(
                      &buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                  for (size_t index = 0; index < accessor.count; index++) {
                     indexBuffer.push_back(buf[index] + vertexStart);
                  }
                  break;
               }
               case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                  auto buf = reinterpret_cast<const uint8_t*>(
                      &buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                  for (size_t index = 0; index < accessor.count; index++) {
                     indexBuffer.push_back(buf[index] + vertexStart);
                  }
                  break;
               }
               default:
                  std::stringstream err;
                  err << "Index component type " << accessor.componentType;
                  throw std::runtime_error(err.str());
            }
         }
      }

      return std::make_unique<Mesh<Models::Vertex, uint32_t>>(
          allocator, vertexBuffer, indexBuffer, transferContext);
   }
   std::stringstream err;
   err << "Error loading model " << filename;
   throw std::runtime_error(err.str());
}
