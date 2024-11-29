#pragma once

#include "cm/Rando.hpp"

#include <cm/Handles.hpp>

namespace tr::gp {

   class AnimationFactory {
    public:
      AnimationFactory() = default;
      ~AnimationFactory() = default;

      AnimationFactory(const AnimationFactory&) = delete;
      auto operator=(const AnimationFactory&) -> AnimationFactory& = delete;

      AnimationFactory(AnimationFactory&&) = delete;
      auto operator=(AnimationFactory&&) -> AnimationFactory& = delete;

      [[nodiscard]] auto getSkeleton(const cm::SkeletonHandle skeletonHandle) const -> const auto& {
         return skeletons.at(skeletonHandle);
      }

      [[nodiscard]] auto getAnimation(const cm::AnimationHandle animationHandle) const -> const
          auto& {
         assert(animations.contains(animationHandle));
         return animations.at(animationHandle);
      }

      auto loadSkeleton(const std::filesystem::path& path) -> cm::SkeletonHandle;
      auto loadAnimation(const std::filesystem::path& path) -> cm::AnimationHandle;

    private:
      cm::MapKey skeletonMapKey{};
      cm::MapKey animationMapKey{};
      std::unordered_map<std::string, cm::AnimationHandle> loadedAnimations;
      std::unordered_map<std::string, cm::SkeletonHandle> loadedSkeletons;

      std::unordered_map<cm::AnimationHandle, ozz::animation::Animation> animations;
      std::unordered_map<cm::SkeletonHandle, ozz::animation::Skeleton> skeletons;
   };
}