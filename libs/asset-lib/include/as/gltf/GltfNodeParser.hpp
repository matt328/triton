#pragma once
#include "as/gltf/TransformParser.hpp"

namespace tr::as {
class TransformParser;
class GeometryExtractor;
class TextureExtractor;
struct Model;
} // namespace tr::as
namespace tinygltf {
class Node;
class Model;
} // namespace tinygltf

namespace tr::as::gltf {

class GltfNodeParser final : public NodeParser {
public:
  explicit GltfNodeParser(std::unique_ptr<as::TransformParser> transformParser,
                          std::unique_ptr<as::GeometryExtractor> geometryExtractor,
                          std::unique_ptr<as::TextureExtractor> textureExtractor);
  ~GltfNodeParser() noexcept override;

  GltfNodeParser(const GltfNodeParser&) = delete;
  auto operator=(const GltfNodeParser&) -> GltfNodeParser& = delete;

  GltfNodeParser(GltfNodeParser&&) = delete;
  auto operator=(GltfNodeParser&&) -> GltfNodeParser& = delete;

  void execute(const tinygltf::Model& model,
               const tinygltf::Node& node,
               Model& tritonModel) override;

private:
  std::unique_ptr<as::TransformParser> transformParser;
  std::unique_ptr<as::GeometryExtractor> geometryExtractor;
  std::unique_ptr<as::TextureExtractor> textureExtractor;
};

} // namespace tr::as::gltf
