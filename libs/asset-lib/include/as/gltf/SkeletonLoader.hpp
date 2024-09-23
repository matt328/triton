#pragma once

#include "as/ConverterComponents.hpp"

namespace tr::as::gltf {

   class SkeletonLoader final : public as::SkeletonLoader {
    public:
      SkeletonLoader() = default;
      ~SkeletonLoader() noexcept override;

      SkeletonLoader(const SkeletonLoader&) = delete;
      auto operator=(const SkeletonLoader&) -> SkeletonLoader& = delete;

      SkeletonLoader(SkeletonLoader&&) = delete;
      auto operator=(SkeletonLoader&&) -> SkeletonLoader& = delete;

      [[nodiscard]] auto load(const std::filesystem::path& path) const
          -> ozz::animation::Skeleton override;
   };

} // namespace tr::as::gltf
