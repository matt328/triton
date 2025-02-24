#pragma once
#include "gp/AssetManager.hpp"
#include "gp/EntityService.hpp"

namespace tr {

class AnimationSystem {
public:
  explicit AnimationSystem(std::shared_ptr<AssetManager> newAssetManager,
                           std::shared_ptr<EntityService> newEntityService);
  ~AnimationSystem() = default;

  AnimationSystem(const AnimationSystem&) = delete;
  AnimationSystem(AnimationSystem&&) = delete;
  auto operator=(const AnimationSystem&) -> AnimationSystem& = delete;
  auto operator=(AnimationSystem&&) -> AnimationSystem& = delete;

  auto update() const -> void;

private:
  std::shared_ptr<AssetManager> assetManager;
  std::shared_ptr<EntityService> entityService;
};

}
