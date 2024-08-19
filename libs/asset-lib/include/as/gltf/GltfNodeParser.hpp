#pragma once
#include "as/gltf/TransformParser.hpp"

namespace tr::as {
   class TransformParser;
   class GeometryExtractor;
   class TextureExtractor;
   struct Model;
}
namespace tinygltf {
   class Node;
   class Model;
}

namespace tr::as::gltf {

   class GltfNodeParser final : public NodeParser {
    public:
      explicit GltfNodeParser(std::unique_ptr<as::TransformParser> transformParser,
                              std::unique_ptr<as::GeometryExtractor> geometryExtractor,
                              std::unique_ptr<as::TextureExtractor> textureExtractor);
      ~GltfNodeParser() noexcept override;

      GltfNodeParser(const GltfNodeParser&) = delete;
      GltfNodeParser& operator=(const GltfNodeParser&) = delete;

      GltfNodeParser(GltfNodeParser&&) = delete;
      GltfNodeParser& operator=(GltfNodeParser&&) = delete;

      void execute(const tinygltf::Model& model,
                   const tinygltf::Node& node,
                   Model& tritonModel) override;

    private:
      std::unique_ptr<as::TransformParser> transformParser;
      std::unique_ptr<as::GeometryExtractor> geometryExtractor;
      std::unique_ptr<as::TextureExtractor> textureExtractor;
   };

}
