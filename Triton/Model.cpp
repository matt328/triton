#include "Model.h"
#include "Vertex.h"

namespace Models {

   Model::Model(const std::string_view& filename) {
      tinygltf::Model gltfModel;
      tinygltf::TinyGLTF gltfContext;

      std::string error, warning;

      if (const bool fileLoaded =
              gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, filename.data())) {
         const tinygltf::Scene& scene =
             gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];

         for (size_t i = 0; i < scene.nodes.size(); i++) {
            std::vector<Vertex> vertexBuffer;
            std::vector<uint32_t> indexBuffer;
            const tinygltf::Node node = gltfModel.nodes[scene.nodes[i]];
            loadNode(nullptr, node, scene.nodes[i], gltfModel, indexBuffer, vertexBuffer, 1.f);
         }

      } else {
         throw std::runtime_error(std::format("Error loading model {}", filename));
      }
   }
}
