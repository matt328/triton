#pragma once

#include "Rando.hpp"
#include "cm/Handles.hpp"

namespace tr {

class AnimationFactory {
public:
  AnimationFactory() = default;
  ~AnimationFactory() = default;

  AnimationFactory(const AnimationFactory&) = delete;
  auto operator=(const AnimationFactory&) -> AnimationFactory& = delete;

  AnimationFactory(AnimationFactory&&) = delete;
  auto operator=(AnimationFactory&&) -> AnimationFactory& = delete;

  [[nodiscard]] auto getSkeleton(const SkeletonHandle skeletonHandle) const -> const auto& {
    return skeletons.at(skeletonHandle);
  }

  [[nodiscard]] auto getAnimation(const AnimationHandle animationHandle) const -> const auto& {
    assert(animations.contains(animationHandle));
    return animations.at(animationHandle);
  }

  auto loadSkeleton(const std::filesystem::path& path) -> SkeletonHandle;
  auto loadAnimation(const std::filesystem::path& path) -> AnimationHandle;

private:
  MapKey skeletonMapKey{};
  MapKey animationMapKey{};
  std::unordered_map<std::string, AnimationHandle> loadedAnimations;
  std::unordered_map<std::string, SkeletonHandle> loadedSkeletons;

  std::unordered_map<AnimationHandle, ozz::animation::Animation> animations;
  std::unordered_map<SkeletonHandle, ozz::animation::Skeleton> skeletons;
};
}
