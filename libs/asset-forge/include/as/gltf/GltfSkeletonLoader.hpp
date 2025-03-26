#pragma once

#include "as/ConverterComponents.hpp"

namespace as {

class GltfSkeletonLoader final : public as::ISkeletonLoader {
public:
  GltfSkeletonLoader() = default;
  ~GltfSkeletonLoader() noexcept override;

  GltfSkeletonLoader(const GltfSkeletonLoader&) = delete;
  auto operator=(const GltfSkeletonLoader&) -> GltfSkeletonLoader& = delete;

  GltfSkeletonLoader(GltfSkeletonLoader&&) = delete;
  auto operator=(GltfSkeletonLoader&&) -> GltfSkeletonLoader& = delete;

  [[nodiscard]] auto load(const std::filesystem::path& path) const
      -> ozz::animation::Skeleton override;
};

} // namespace as
