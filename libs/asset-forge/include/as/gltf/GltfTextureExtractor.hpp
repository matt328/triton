#pragma once

#include "as/ConverterComponents.hpp"

namespace as {
struct Model;
} // namespace as

namespace tinygltf {
class Model;
} // namespace tinygltf

namespace as {

class GltfTextureExtractor final : public as::ITextureExtractor {
public:
  GltfTextureExtractor() = default;
  ~GltfTextureExtractor() noexcept override;

  GltfTextureExtractor(const GltfTextureExtractor&) = delete;
  auto operator=(const GltfTextureExtractor&) -> GltfTextureExtractor& = delete;

  GltfTextureExtractor(GltfTextureExtractor&&) = delete;
  auto operator=(GltfTextureExtractor&&) -> GltfTextureExtractor& = delete;

  void execute(const tinygltf::Model& model, int textureIndex, Model& tritonModel) override;
};

} // namespace as
