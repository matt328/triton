#pragma once
#include "as/ConverterComponents.hpp"

namespace as {

class GltfGeometryExtractor final : public as::IGeometryExtractor {
public:
  GltfGeometryExtractor() = default;
  ~GltfGeometryExtractor() noexcept override;

  GltfGeometryExtractor(const GltfGeometryExtractor&) = delete;
  auto operator=(const GltfGeometryExtractor&) -> GltfGeometryExtractor& = delete;

  GltfGeometryExtractor(GltfGeometryExtractor&&) = delete;
  auto operator=(GltfGeometryExtractor&&) -> GltfGeometryExtractor& = delete;

  void execute(const tinygltf::Model& model,
               const tinygltf::Primitive& primitive,
               const glm::mat4& transform,
               Model& tritonModel) override;
};

}
