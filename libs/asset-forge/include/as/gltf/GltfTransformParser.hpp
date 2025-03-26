#pragma once

#include "as/ConverterComponents.hpp"

namespace tinygltf {
class Node;
} // namespace tinygltf
namespace as {

class GltfTransformParser final : public as::ITransformParser {
public:
  GltfTransformParser() = default;
  ~GltfTransformParser() noexcept override;

  GltfTransformParser(const GltfTransformParser&) = delete;
  auto operator=(const GltfTransformParser&) -> GltfTransformParser& = delete;

  GltfTransformParser(GltfTransformParser&&) = delete;
  auto operator=(GltfTransformParser&&) -> GltfTransformParser& = delete;

  [[nodiscard]] auto execute(const tinygltf::Node& node) const -> glm::mat4 override;
};

} // namespace as
