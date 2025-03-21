#pragma once

#include "fx/IAssetService.hpp"
#include "cm/Rando.hpp"

namespace tr {

struct TritonModelData;

class DefaultAssetService : public IAssetService {
public:
  DefaultAssetService() = default;
  ~DefaultAssetService() override = default;

  DefaultAssetService(const DefaultAssetService&) = delete;
  DefaultAssetService(DefaultAssetService&&) = delete;
  auto operator=(const DefaultAssetService&) -> DefaultAssetService& = delete;
  auto operator=(DefaultAssetService&&) -> DefaultAssetService& = delete;

  auto loadModel(std::string_view filename) -> as::Model override;
  auto loadSkeleton(std::string_view filename) -> SkeletonHandle override;
  auto loadAnimation(std::string_view filename) -> AnimationHandle override;

  auto getAnimation(AnimationHandle handle) -> const ozz::animation::Animation& override;
  auto getSkeleton(SkeletonHandle handle) -> const ozz::animation::Skeleton& override;

private:
  MapKey skeletonKey;
  MapKey animationKey;

  std::unordered_map<std::string, AnimationHandle> loadedAnimations;
  std::unordered_map<std::string, SkeletonHandle> loadedSkeletons;

  std::unordered_map<AnimationHandle, ozz::animation::Animation> animations;
  std::unordered_map<SkeletonHandle, ozz::animation::Skeleton> skeletons;
};

}
