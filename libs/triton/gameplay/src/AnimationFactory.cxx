#include "AnimationFactory.hpp"

namespace tr::gp {
   auto AnimationFactory::loadSkeleton(const std::filesystem::path& path) -> SkeletonHandle {
      const auto filename = path.string();

      if (const auto it = loadedSkeletons.find(filename); it != loadedSkeletons.end()) {
         return it->second;
      }

      ozz::io::File file(filename.c_str(), "rb");

      if (!file.opened()) {
         Log.error("Failed to open skeleton file: {0}.", filename);
      }

      ozz::io::IArchive archive(&file);
      if (!archive.TestTag<ozz::animation::Skeleton>()) {
         Log.error("Failed to load skeleton instance from file: {0}", filename);
      }

      auto skeleton = ozz::animation::Skeleton{};
      archive >> skeleton;

      const auto key = skeletonMapKey.getKey();

      skeletons.emplace(key, std::move(skeleton));

      loadedSkeletons.emplace(filename, key);

      return key;
   }

   auto AnimationFactory::loadAnimation(const std::filesystem::path& path) -> AnimationHandle {
      const auto filename = path.string();

      if (const auto it = loadedAnimations.find(filename); it != loadedAnimations.end()) {
         return it->second;
      }
      ozz::io::File file(filename.c_str(), "rb");
      if (!file.opened()) {
         Log.error("Failed to open animation file: {0}", filename);
      }
      ozz::io::IArchive archive(&file);
      if (!archive.TestTag<ozz::animation::Animation>()) {
         Log.error("Failed to load animation from file: {0}", filename);
      }
      auto animation = ozz::animation::Animation{};
      archive >> animation;

      const auto key = animationMapKey.getKey();
      animations.emplace(key, std::move(animation));

      loadedAnimations.emplace(filename, key);

      return key;
   }
}