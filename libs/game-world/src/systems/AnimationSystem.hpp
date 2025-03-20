#pragma once
#include "fx/IAssetService.hpp"
#include "EntityService.hpp"

namespace tr {

class AnimationSystem {
public:
  explicit AnimationSystem(std::shared_ptr<IAssetService> assetService,
                           std::shared_ptr<EntityService> newEntityService);
  ~AnimationSystem() = default;

  AnimationSystem(const AnimationSystem&) = delete;
  AnimationSystem(AnimationSystem&&) = delete;
  auto operator=(const AnimationSystem&) -> AnimationSystem& = delete;
  auto operator=(AnimationSystem&&) -> AnimationSystem& = delete;

  auto update() const -> void;

private:
  std::shared_ptr<IAssetService> assetService;
  std::shared_ptr<EntityService> entityService;

  [[nodiscard]] auto convertOzzToGlm(const ozz::math::Float4x4& ozzMatrix) const -> glm::mat4;
};

}
