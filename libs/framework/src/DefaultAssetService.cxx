#include "DefaultAssetService.hpp"

#include "as/Model.hpp"
#include "api/SharedExceptions.hpp"

namespace tr {

auto DefaultAssetService::loadModel(std::string_view filename) -> as::Model {
  try {
    auto modelPath = std::filesystem::path{filename};
    ZoneNamedN(z, "Loading Model from Disk", true);
    if (modelPath.extension() == ".trm") {
      auto is = std::ifstream(modelPath, std::ios::binary);
      if (!is) {
        throw IOException("Failed to open file: " + modelPath.string());
      }

      try {
        cereal::PortableBinaryInputArchive input(is);
        auto tritonModel = as::Model{};
        input(tritonModel);
        is.close();
        return tritonModel;
      } catch (const std::exception& ex) {
        throw IOException("Error reading: " + modelPath.string() + ": ", ex);
      }
    } else if (modelPath.extension() == ".json") {
      auto is = std::ifstream(modelPath);
      if (!is) {
        throw IOException("Failed to open file: " + modelPath.string());
      }
      try {
        cereal::JSONInputArchive input(is);
        auto tritonModel = as::Model{};
        input(tritonModel);
        return tritonModel;
      } catch (const std::exception& ex) {
        throw IOException("Error reading: " + modelPath.string() + ": ", ex);
      }
    }
  } catch (BaseException& ex) {
    ex << "DefaultAssetService::loadModel(): ";
    throw;
  }
  Log.warn("loadTrmFile returning empty model");
  return {};
}

auto DefaultAssetService::loadSkeleton(std::string_view filename) -> SkeletonHandle {
  if (const auto it = loadedSkeletons.find(filename.data()); it != loadedSkeletons.end()) {
    return it->second;
  }

  ozz::io::File file(filename.data(), "rb");

  if (!file.opened()) {
    Log.error("Failed to open skeleton file: {0}.", filename);
  }

  ozz::io::IArchive archive(&file);
  if (!archive.TestTag<ozz::animation::Skeleton>()) {
    Log.error("Failed to load skeleton instance from file: {0}", filename);
  }

  auto skeleton = ozz::animation::Skeleton{};
  archive >> skeleton;

  const auto key = skeletonKey.getKey();

  skeletons.emplace(key, std::move(skeleton));

  loadedSkeletons.emplace(filename, key);

  return key;
}

auto DefaultAssetService::loadAnimation(std::string_view filename) -> AnimationHandle {
  if (loadedAnimations.contains(filename.data())) {
    return loadedAnimations.at(filename.data());
  }

  ozz::io::File file(filename.data(), "rb");
  if (!file.opened()) {
    Log.error("Failed to open animation file: {0}", filename);
  }

  ozz::io::IArchive archive(&file);
  if (!archive.TestTag<ozz::animation::Animation>()) {
    Log.error("Failed to load animation from file: {0}", filename);
  }

  auto animation = ozz::animation::Animation{};
  archive >> animation;

  const auto key = animationKey.getKey();
  animations.emplace(key, std::move(animation));

  loadedAnimations.emplace(filename, key);

  return key;
}

auto DefaultAssetService::getAnimation(AnimationHandle handle) -> const ozz::animation::Animation& {
  return animations.at(handle);
}

auto DefaultAssetService::getSkeleton(SkeletonHandle handle) -> const ozz::animation::Skeleton& {
  return skeletons.at(handle);
}

}
