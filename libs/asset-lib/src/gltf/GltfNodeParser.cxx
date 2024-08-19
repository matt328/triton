#include "as/gltf/GltfNodeParser.hpp"

#include <tiny_gltf.h>

#include "as/ConverterComponents.hpp"

namespace tr::as::gltf {
   GltfNodeParser::GltfNodeParser(std::unique_ptr<as::TransformParser> transformParser,
                                  std::unique_ptr<as::GeometryExtractor> geometryExtractor,
                                  std::unique_ptr<as::TextureExtractor> textureExtractor)
       : transformParser{std::move(transformParser)},
         geometryExtractor{std::move(geometryExtractor)},
         textureExtractor{std::move(textureExtractor)} {
   }

   GltfNodeParser::~GltfNodeParser() noexcept {
   }

   void GltfNodeParser::execute(const tinygltf::Model& model,
                                const tinygltf::Node& node,
                                Model& tritonModel) {
      if (node.mesh != -1) {
         const auto nodeTransform = transformParser->execute(node);
         for (const auto& mesh = model.meshes[node.mesh]; const auto& primitive : mesh.primitives) {

            geometryExtractor->execute(model, primitive, nodeTransform, tritonModel);

            const auto materialIndex = primitive.material;
            const auto& material = model.materials[materialIndex];
            const auto& baseColorTextureIndex =
                material.pbrMetallicRoughness.baseColorTexture.index;
            textureExtractor->execute(model, baseColorTextureIndex, tritonModel);
         }
      }
      for (auto& child : node.children) {
         auto& localNode = model.nodes[child];
         execute(model, localNode, tritonModel);
      }
   }
}
