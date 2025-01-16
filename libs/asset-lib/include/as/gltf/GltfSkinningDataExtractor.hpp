#pragma once

#include "as/ConverterComponents.hpp"

namespace as {

class GltfSkinningDataExtractor final : public as::ISkinningDataExtractor {
public:
  GltfSkinningDataExtractor() = default;
  ~GltfSkinningDataExtractor() noexcept override;

  GltfSkinningDataExtractor(const GltfSkinningDataExtractor&) = delete;
  auto operator=(const GltfSkinningDataExtractor&) -> GltfSkinningDataExtractor& = delete;

  GltfSkinningDataExtractor(GltfSkinningDataExtractor&&) = delete;
  auto operator=(GltfSkinningDataExtractor&&) -> GltfSkinningDataExtractor& = delete;

  void execute(const tinygltf::Model& model,
               const ozz::animation::Skeleton& skeleton,
               Model& tritonModel) override;
};

}
