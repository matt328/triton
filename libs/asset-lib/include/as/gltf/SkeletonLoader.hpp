#pragma once

#include "as/ConverterComponents.hpp"

namespace tr::as::gltf {

   class SkeletonLoader final : public as::SkeletonLoader {
    public:
      SkeletonLoader() = default;
      ~SkeletonLoader() noexcept override;

      SkeletonLoader(const SkeletonLoader&) = delete;
      SkeletonLoader& operator=(const SkeletonLoader&) = delete;

      SkeletonLoader(SkeletonLoader&&) = delete;
      SkeletonLoader& operator=(SkeletonLoader&&) = delete;

      auto load(const std::filesystem::path& path) const -> ozz::animation::Skeleton override;
   };

}
