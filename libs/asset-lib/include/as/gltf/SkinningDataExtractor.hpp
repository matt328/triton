#pragma once

#include "as/ConverterComponents.hpp"

namespace tr::as::gltf {

class SkinningDataExtractor final : public as::SkinningDataExtractor {
public:
  SkinningDataExtractor() = default;
  ~SkinningDataExtractor() noexcept override;

  SkinningDataExtractor(const SkinningDataExtractor&) = delete;
  auto operator=(const SkinningDataExtractor&) -> SkinningDataExtractor& = delete;

  SkinningDataExtractor(SkinningDataExtractor&&) = delete;
  auto operator=(SkinningDataExtractor&&) -> SkinningDataExtractor& = delete;

  void execute(const tinygltf::Model& model,
               const ozz::animation::Skeleton& skeleton,
               Model& tritonModel) override;
};

} // namespace tr::as::gltf
