#include "AnimationFactory.hpp"

namespace tr::gfx::geo {
   auto AnimationFactory::loadSkeleton(const std::filesystem::path& path) -> SkeletonHandle {
      const auto filename = path.string();

      const auto it = loadedSkeletons.find(filename);
      if (it != loadedSkeletons.end()) {
         return it->second;
      }

      ozz::io::File file(filename.c_str(), "rb");

      if (!file.opened()) {
         Log::error << "Failed to open skeleton file " << filename << "." << std::endl;
      }

      ozz::io::IArchive archive(&file);
      if (!archive.TestTag<ozz::animation::Skeleton>()) {
         Log::error << "Failed to load skeleton instance from file " << filename << "."
                    << std::endl;
      }

      auto skeleton = ozz::animation::Skeleton{};
      archive >> skeleton;

      for (const auto& name : skeleton.joint_names()) {
         Log::debug << "skeleton joint name: " << name << std::endl;
      }

      const auto key = skeletonMapKey.getKey();

      skeletons.emplace(key, std::move(skeleton));

      loadedSkeletons.emplace(filename, key);

      return key;
   }

   auto AnimationFactory::loadAnimation(const std::filesystem::path& path) -> AnimationHandle {
      const auto filename = path.string();

      const auto it = loadedAnimations.find(filename);
      if (it != loadedAnimations.end()) {
         return it->second;
      }
      ozz::io::File file(filename.c_str(), "rb");
      if (!file.opened()) {
         Log::error << "Failed to open animation file " << filename << "." << std::endl;
      }
      ozz::io::IArchive archive(&file);
      if (!archive.TestTag<ozz::animation::Animation>()) {
         Log::error << "Failed to load animation instance from file " << filename << "."
                    << std::endl;
      }
      auto animation = ozz::animation::Animation{};
      archive >> animation;

      const auto key = animationMapKey.getKey();
      animations.emplace(key, std::move(animation));

      loadedAnimations.emplace(filename, key);

      return key;
   }
}