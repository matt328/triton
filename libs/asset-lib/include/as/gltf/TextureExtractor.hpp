#pragma once

#include "as/ConverterComponents.hpp"

namespace tr::as {
struct Model;
} // namespace tr::as

namespace tinygltf {
class Model;
} // namespace tinygltf

namespace tr::as::gltf {

class TextureExtractor final : public as::TextureExtractor {
public:
  TextureExtractor() = default;
  ~TextureExtractor() noexcept override;

  TextureExtractor(const TextureExtractor&) = delete;
  auto operator=(const TextureExtractor&) -> TextureExtractor& = delete;

  TextureExtractor(TextureExtractor&&) = delete;
  auto operator=(TextureExtractor&&) -> TextureExtractor& = delete;

  void execute(const tinygltf::Model& model, int textureIndex, Model& tritonModel) override;
};

} // namespace tr::as::gltf
