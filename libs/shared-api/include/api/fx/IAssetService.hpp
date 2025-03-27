#pragma once

#include "api/gw/Handles.hpp"

namespace as {
struct Model;
}

namespace tr {

class IGeometryData;

class IAssetService {
public:
  IAssetService() = default;
  virtual ~IAssetService() = default;

  IAssetService(const IAssetService&) = default;
  IAssetService(IAssetService&&) = delete;
  auto operator=(const IAssetService&) -> IAssetService& = default;
  auto operator=(IAssetService&&) -> IAssetService& = delete;

  virtual auto loadModel(std::string_view filename) -> as::Model = 0;
  virtual auto loadSkeleton(std::string_view filename) -> SkeletonHandle = 0;
  virtual auto loadAnimation(std::string_view filename) -> AnimationHandle = 0;

  virtual auto getAnimation(AnimationHandle handle) -> const ozz::animation::Animation& = 0;
  virtual auto getSkeleton(SkeletonHandle handle) -> const ozz::animation::Skeleton& = 0;
};

}
