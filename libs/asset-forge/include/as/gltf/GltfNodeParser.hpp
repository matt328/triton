#pragma once
#include "as/gltf/GltfTransformParser.hpp"

namespace as {
class ITransformParser;
class IGeometryExtractor;
class ITextureExtractor;
struct Model;
} // namespace as
namespace tinygltf {
class Node;
class Model;
} // namespace tinygltf

namespace as {

class GltfNodeParser final : public INodeParser {
public:
  explicit GltfNodeParser(std::unique_ptr<as::ITransformParser> transformParser,
                          std::unique_ptr<as::IGeometryExtractor> geometryExtractor,
                          std::unique_ptr<as::ITextureExtractor> textureExtractor);
  ~GltfNodeParser() noexcept override;

  GltfNodeParser(const GltfNodeParser&) = delete;
  auto operator=(const GltfNodeParser&) -> GltfNodeParser& = delete;

  GltfNodeParser(GltfNodeParser&&) = delete;
  auto operator=(GltfNodeParser&&) -> GltfNodeParser& = delete;

  void execute(const tinygltf::Model& model,
               const tinygltf::Node& node,
               Model& tritonModel) override;

private:
  std::unique_ptr<as::ITransformParser> transformParser;
  std::unique_ptr<as::IGeometryExtractor> geometryExtractor;
  std::unique_ptr<as::ITextureExtractor> textureExtractor;
};

} // namespace as
