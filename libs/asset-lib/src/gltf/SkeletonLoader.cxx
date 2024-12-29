#include "as/gltf/SkeletonLoader.hpp"

namespace tr::as::gltf {

SkeletonLoader::~SkeletonLoader() noexcept {
}

auto SkeletonLoader::load(const std::filesystem::path& path) const -> ozz::animation::Skeleton {
  ozz::io::File file(path.string().c_str(), "rb");

  if (!file.opened()) {
    Log.error("Failed to open skeleton file {0}", path.string());
  }

  ozz::io::IArchive archive(&file);
  if (!archive.TestTag<ozz::animation::Skeleton>()) {
    Log.error("Failed to load skeleton instance from file {0}", path.string());
  }

  auto ozzSkeleton = ozz::animation::Skeleton{};
  archive >> ozzSkeleton;
  return ozzSkeleton;
}

}
