#pragma once

#include "gfx/helpers/Rando.hpp"

namespace tr::gfx::geo {

   using AnimationHandle = uint64_t;
   using SkeletonHandle = uint64_t;

   class AnimationFactory {
    public:
      AnimationFactory() = default;
      ~AnimationFactory() = default;

      AnimationFactory(const AnimationFactory&) = delete;
      AnimationFactory& operator=(const AnimationFactory&) = delete;

      AnimationFactory(AnimationFactory&&) = delete;
      AnimationFactory& operator=(AnimationFactory&&) = delete;

      [[nodiscard]] const auto& getSkeleton(const SkeletonHandle skeletonHandle) const {
         return skeletons.at(skeletonHandle);
      }

      [[nodiscard]] const auto& getAnimation(const AnimationHandle animationHandle) const {
         return animations.at(animationHandle);
      }

      auto loadSkeleton(const std::filesystem::path& path) -> SkeletonHandle;
      auto loadAnimation(const std::filesystem::path& path) -> AnimationHandle;

    private:
      rng::MapKey skeletonMapKey{};
      rng::MapKey animationMapKey{};
      std::unordered_map<std::string, AnimationHandle> loadedAnimations;
      std::unordered_map<std::string, SkeletonHandle> loadedSkeletons;

      std::unordered_map<AnimationHandle, ozz::animation::Animation> animations;
      std::unordered_map<SkeletonHandle, ozz::animation::Skeleton> skeletons;
   };
}
